#include "OptoCompressor.h"
#include <cmath>

OptoCompressor::OptoCompressor()
{
}

void OptoCompressor::prepare(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;
    updateCoefficients();

    // Meter smoothing: ~100ms time constant
    meterSmoothingCoeff = std::exp(-1.0f / (0.1f * static_cast<float>(sampleRate)));

    reset();
}

void OptoCompressor::reset()
{
    optoCellState = 0.0f;
    fastReleaseEnv = 0.0f;
    slowReleaseEnv = 0.0f;
    adaptiveReleaseTime = MIN_SLOW_RELEASE_MS;
    smoothedGR = 0.0f;
    smoothedOutput = 0.0f;
}

void OptoCompressor::updateCoefficients()
{
    // Attack coefficient (program-dependent, but base value)
    float attackMs = BASE_ATTACK_MS;
    attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * static_cast<float>(sampleRate)));

    // Fast release: fixed 60ms
    fastReleaseCoeff = std::exp(-1.0f / (FAST_RELEASE_MS * 0.001f * static_cast<float>(sampleRate)));

    // Slow release: adaptive, start at minimum
    float slowMs = adaptiveReleaseTime;
    slowReleaseCoeff = std::exp(-1.0f / (slowMs * 0.001f * static_cast<float>(sampleRate)));
}

void OptoCompressor::setPeakReduction(float value)
{
    peakReduction = juce::jlimit(0.0f, 100.0f, value);
}

void OptoCompressor::setGain(float dB)
{
    makeupGain = juce::Decibels::decibelsToGain(juce::jlimit(-10.0f, 40.0f, dB));
}

void OptoCompressor::setLimitMode(bool limit)
{
    limitMode = limit;
}

void OptoCompressor::setMix(float percent)
{
    mix = juce::jlimit(0.0f, 100.0f, percent) / 100.0f;
}

float OptoCompressor::computeGain(float inputLevelDb)
{
    if (peakReduction <= 0.0f)
        return 1.0f;

    // Threshold derived from peak reduction
    // Higher peak reduction = lower threshold = more compression
    float threshold = 0.0f - (peakReduction * 0.4f); // -40dB at max

    // Ratio based on mode
    float ratio = limitMode ? LIMIT_RATIO : COMPRESS_RATIO;

    // Soft knee computation
    float kneeStart = threshold - KNEE_WIDTH_DB / 2.0f;
    float kneeEnd = threshold + KNEE_WIDTH_DB / 2.0f;

    float gainReductionDb = 0.0f;

    if (inputLevelDb <= kneeStart)
    {
        // Below knee - no compression
        gainReductionDb = 0.0f;
    }
    else if (inputLevelDb >= kneeEnd)
    {
        // Above knee - full compression
        float excess = inputLevelDb - threshold;
        gainReductionDb = excess * (1.0f - 1.0f / ratio);
    }
    else
    {
        // In knee - gradual compression
        float kneePosition = (inputLevelDb - kneeStart) / KNEE_WIDTH_DB;
        float kneeGain = kneePosition * kneePosition;
        float excess = inputLevelDb - threshold;
        gainReductionDb = kneeGain * excess * (1.0f - 1.0f / ratio);
    }

    return juce::Decibels::decibelsToGain(-gainReductionDb);
}

float OptoCompressor::processOpticalCell(float targetGain)
{
    // The optical cell has inertia - it can't change instantly
    // Attack is faster when signal is sustained (program-dependent)
    // Release has two stages: fast initial, slow tail

    float currentGainDb = juce::Decibels::gainToDecibels(optoCellState + 0.0001f);
    float targetGainDb = juce::Decibels::gainToDecibels(targetGain + 0.0001f);

    if (targetGain < optoCellState)
    {
        // Attacking (gain reduction increasing)
        // Program-dependent: faster attack for sustained signals
        float attackSpeed = attackCoeff;

        // Adapt attack based on how long we've been compressing
        if (slowReleaseEnv > 0.5f)
        {
            // Sustained compression - speed up attack
            attackSpeed = std::exp(-1.0f / (BASE_ATTACK_MS * 0.5f * 0.001f * static_cast<float>(sampleRate)));
        }

        optoCellState = attackSpeed * optoCellState + (1.0f - attackSpeed) * targetGain;
    }
    else
    {
        // Releasing (gain reduction decreasing)
        // Two-stage release: fast initial + slow tail

        // Fast release envelope
        fastReleaseEnv = fastReleaseCoeff * fastReleaseEnv + (1.0f - fastReleaseCoeff) * targetGain;

        // Adaptive slow release time based on how much compression occurred
        float compressionDepth = 1.0f - optoCellState;
        adaptiveReleaseTime = MIN_SLOW_RELEASE_MS +
            compressionDepth * (MAX_SLOW_RELEASE_MS - MIN_SLOW_RELEASE_MS);

        // Update slow release coefficient
        slowReleaseCoeff = std::exp(-1.0f / (adaptiveReleaseTime * 0.001f * static_cast<float>(sampleRate)));

        // Slow release envelope
        slowReleaseEnv = slowReleaseCoeff * slowReleaseEnv + (1.0f - slowReleaseCoeff) * targetGain;

        // Combine: 40% fast, 60% slow (LA-2A characteristic)
        optoCellState = 0.4f * fastReleaseEnv + 0.6f * slowReleaseEnv;
    }

    return optoCellState;
}

void OptoCompressor::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    float maxGR = 0.0f;
    float maxOutput = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get input level (sum channels for stereo linking)
        float inputLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s = buffer.getSample(ch, sample);
            inputLevel += s * s;
        }
        inputLevel = std::sqrt(inputLevel / static_cast<float>(numChannels));

        // Convert to dB
        float inputLevelDb = juce::Decibels::gainToDecibels(inputLevel + 0.0001f);

        // Compute target gain from compression curve
        float targetGain = computeGain(inputLevelDb);

        // Process through optical cell (adds attack/release characteristics)
        float gain = processOpticalCell(targetGain);

        // Track gain reduction for metering
        float grDb = juce::Decibels::gainToDecibels(gain);
        if (grDb < maxGR)
            maxGR = grDb;

        // Apply gain to all channels
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float dry = buffer.getSample(ch, sample);
            float wet = dry * gain * makeupGain;

            // Mix dry/wet
            float output = dry * (1.0f - mix) + wet * mix;
            buffer.setSample(ch, sample, output);

            // Track output level
            float outAbs = std::abs(output);
            if (outAbs > maxOutput)
                maxOutput = outAbs;
        }
    }

    // Update meters with smoothing
    smoothedGR = meterSmoothingCoeff * smoothedGR + (1.0f - meterSmoothingCoeff) * maxGR;
    smoothedOutput = meterSmoothingCoeff * smoothedOutput + (1.0f - meterSmoothingCoeff) * maxOutput;

    currentGainReductionDb.store(smoothedGR);
    currentOutputLevel.store(juce::Decibels::gainToDecibels(smoothedOutput + 0.0001f));
}
