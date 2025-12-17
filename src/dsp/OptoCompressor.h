#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>

/**
 * T4B Opto-Cell Compressor Model
 *
 * Simulates the LA-2A's electro-optical attenuator behavior:
 * - Program-dependent attack (10-100ms based on transient content)
 * - Two-stage release (fast 60ms + slow 1-15s adaptive)
 * - Soft knee compression curve
 * - Limit mode (high ratio) vs Compress mode (3:1)
 */
class OptoCompressor
{
public:
    OptoCompressor();
    ~OptoCompressor() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void processBlock(juce::AudioBuffer<float>& buffer);

    // Parameters
    void setPeakReduction(float value);    // 0-100
    void setGain(float dB);                // -10 to +40
    void setLimitMode(bool limit);         // true = limit, false = compress
    void setBritishMode(bool british);     // 1176-style all-buttons-in
    void setMix(float percent);            // 0-100

    // Metering (thread-safe)
    float getGainReductionDb() const { return currentGainReductionDb.load(); }
    float getOutputLevel() const { return currentOutputLevel.load(); }

private:
    double sampleRate = 44100.0;

    // Opto-cell state variables (per channel for stereo linking)
    float optoCellState = 1.0f;      // Start at unity gain
    float fastReleaseEnv = 1.0f;     // Start at unity gain
    float slowReleaseEnv = 1.0f;     // Start at unity gain

    // Adaptive timing state
    float attackCoeff = 0.0f;
    float fastReleaseCoeff = 0.0f;
    float slowReleaseCoeff = 0.0f;
    float adaptiveReleaseTime = 1.0f;

    // Parameters
    float peakReduction = 0.0f;
    float makeupGain = 1.0f;
    bool limitMode = false;
    bool britishMode = false;
    float mix = 1.0f;

    // Metering
    std::atomic<float> currentGainReductionDb{0.0f};
    std::atomic<float> currentOutputLevel{0.0f};
    float meterSmoothingCoeff = 0.0f;
    float smoothedGR = 0.0f;
    float smoothedOutput = 0.0f;

    // Internal methods
    float computeGain(float inputLevel);
    float processOpticalCell(float targetGain);
    void updateCoefficients();

    // Constants
    static constexpr float BASE_ATTACK_MS = 10.0f;
    static constexpr float FAST_RELEASE_MS = 60.0f;
    static constexpr float MIN_SLOW_RELEASE_MS = 1000.0f;
    static constexpr float MAX_SLOW_RELEASE_MS = 15000.0f;
    static constexpr float COMPRESS_RATIO = 3.0f;
    static constexpr float LIMIT_RATIO = 100.0f;
    static constexpr float BRITISH_RATIO = 20.0f;  // 1176 all-buttons-in style
    static constexpr float KNEE_WIDTH_DB = 6.0f;
};
