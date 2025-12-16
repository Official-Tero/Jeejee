# DSP Design: T4 Opto-Cell Modeling

## Overview

The LA-2A uses a Teletronix T4B electro-optical attenuator - a combination of an electroluminescent panel and a photoresistor (photocell). This creates the LA-2A's signature "program-dependent" compression behavior.

## T4B Characteristics

### Physical Behavior

```
Audio Signal → Amplifier → EL Panel (light) → Photocell → Resistance Change → Attenuation
```

The photocell has inherent **capacitance** which creates:
1. **Slow light-to-resistance response** - the cell can't change instantly
2. **Asymmetric attack/release** - attack is generally faster than release
3. **Program-dependent timing** - behavior changes based on signal history

### Key Specifications (from original hardware)

| Parameter | Value |
|-----------|-------|
| Attack time | 10ms (fast transients) to 100ms (sustained) |
| Release (fast stage) | ~60ms |
| Release (slow stage) | 1-15 seconds (program dependent) |
| Compression ratio | ~3:1 (Compress mode) |
| Limiting ratio | ~100:1 (Limit mode) |
| Knee | Soft (gradual ratio increase) |

## Algorithm Design

### State Variables

```cpp
class OptoCompressor {
    // Optical cell simulation
    float optoCellState = 0.0f;      // Current "charge" of the photocell

    // Two-stage release envelope
    float fastReleaseEnv = 0.0f;     // Fast decay (~60ms)
    float slowReleaseEnv = 0.0f;     // Slow decay (1-15s adaptive)

    // Program-dependent adaptation
    float signalHistory = 0.0f;      // Running average for adaptation

    // Metering (thread-safe)
    std::atomic<float> currentGainReduction{0.0f};
    std::atomic<float> currentOutputLevel{0.0f};
};
```

### Processing Pipeline

```
Input Sample
     │
     ▼
┌────────────────────────┐
│ 1. Level Detection     │
│    - RMS with optical  │
│      response curve    │
└──────────┬─────────────┘
           │
           ▼
┌────────────────────────┐
│ 2. Attack/Release      │
│    - Program-dependent │
│      time constants    │
└──────────┬─────────────┘
           │
           ▼
┌────────────────────────┐
│ 3. Gain Computation    │
│    - Soft knee curve   │
│    - Compress/Limit    │
└──────────┬─────────────┘
           │
           ▼
┌────────────────────────┐
│ 4. Gain Application    │
│    - Makeup gain       │
│    - Dry/wet mix       │
└──────────┬─────────────┘
           │
           ▼
Output Sample
```

### 1. Level Detection

The optical cell responds to RMS energy, not instantaneous peaks:

```cpp
float detectLevel(float input) {
    // Simplified RMS approximation using optical response
    float absInput = std::abs(input);

    // Optical cell has inertia - smooth the detection
    float opticalCoeff = 0.995f;  // ~100ms at 44.1kHz
    optoCellState = opticalCoeff * optoCellState + (1.0f - opticalCoeff) * absInput;

    return optoCellState;
}
```

### 2. Program-Dependent Attack/Release

The LA-2A's defining characteristic - timing adapts to the signal:

```cpp
void updateEnvelope(float detectedLevel) {
    // Calculate program-dependent coefficients
    float attackCoeff = calculateAttackCoeff(detectedLevel);
    float releaseCoeff = calculateReleaseCoeff();

    if (detectedLevel > fastReleaseEnv) {
        // Attack phase
        fastReleaseEnv += attackCoeff * (detectedLevel - fastReleaseEnv);
    } else {
        // Release phase - two stages
        fastReleaseEnv *= releaseCoeff;
    }

    // Slow release adapts based on signal history
    slowReleaseEnv = 0.9999f * slowReleaseEnv + 0.0001f * fastReleaseEnv;
}
```

**Attack time calculation:**
```cpp
float calculateAttackCoeff(float level) {
    // Higher sustained levels = faster attack
    // Transients (sudden peaks) = slower attack
    float baseAttack = 0.01f;  // ~10ms
    float maxAttack = 0.1f;    // ~100ms

    // Transient detection: compare instantaneous to history
    float transientFactor = std::max(0.0f, level - signalHistory) / (level + 0.001f);

    // More transients = slower attack (higher time constant)
    float attackTime = baseAttack + transientFactor * (maxAttack - baseAttack);

    return 1.0f - std::exp(-1.0f / (attackTime * sampleRate));
}
```

**Release time calculation:**
```cpp
float calculateReleaseCoeff() {
    // Two-stage release combined
    float fastTime = 0.06f;   // 60ms
    float slowTime = 1.0f + 14.0f * signalHistory;  // 1-15s based on history

    float fastCoeff = std::exp(-1.0f / (fastTime * sampleRate));
    float slowCoeff = std::exp(-1.0f / (slowTime * sampleRate));

    // Blend: 40% fast, 60% slow
    return 0.4f * fastCoeff + 0.6f * slowCoeff;
}
```

### 3. Gain Computation

Soft-knee compression curve:

```cpp
float computeGain(float envelope, float threshold, float ratio, float knee) {
    float inputDb = 20.0f * std::log10(envelope + 1e-10f);
    float outputDb = inputDb;

    // Knee region
    float kneeStart = threshold - knee / 2.0f;
    float kneeEnd = threshold + knee / 2.0f;

    if (inputDb < kneeStart) {
        // Below knee - no compression
        outputDb = inputDb;
    } else if (inputDb > kneeEnd) {
        // Above knee - full compression
        outputDb = threshold + (inputDb - threshold) / ratio;
    } else {
        // In knee - gradual transition
        float kneePosition = (inputDb - kneeStart) / knee;
        float kneeRatio = 1.0f + (ratio - 1.0f) * kneePosition;
        outputDb = threshold + (inputDb - threshold) / kneeRatio;
    }

    float gainDb = outputDb - inputDb;
    return std::pow(10.0f, gainDb / 20.0f);
}
```

### 4. Compress vs Limit Mode

The LA-2A has two modes controlled by a switch:

| Mode | Ratio | Knee | Character |
|------|-------|------|-----------|
| Compress | ~3:1 | Soft (6dB) | Musical, transparent |
| Limit | ~100:1 | Harder (3dB) | Aggressive, brick-wall |

```cpp
float ratio = limitMode ? 100.0f : 3.0f;
float knee = limitMode ? 3.0f : 6.0f;
```

### 5. Peak Reduction Mapping

The "Peak Reduction" control maps to internal threshold:

```cpp
// Peak Reduction 0-100 maps to threshold
// Higher peak reduction = lower threshold = more compression
float threshold = juce::jmap(peakReduction, 0.0f, 100.0f, 0.0f, -40.0f);
```

## Complete Sample Processing

```cpp
float OptoCompressor::processSample(float input) {
    // 1. Detect level with optical characteristics
    float level = detectLevel(input);

    // 2. Update program-dependent envelope
    updateEnvelope(level);

    // 3. Combined envelope (fast + slow blend)
    float envelope = 0.4f * fastReleaseEnv + 0.6f * slowReleaseEnv;

    // 4. Compute gain reduction
    float threshold = juce::jmap(peakReduction, 0.0f, 100.0f, 0.0f, -40.0f);
    float ratio = limitMode ? 100.0f : 3.0f;
    float knee = limitMode ? 3.0f : 6.0f;
    float gain = computeGain(envelope, threshold, ratio, knee);

    // 5. Apply compression
    float compressed = input * gain;

    // 6. Apply makeup gain
    float makeupLinear = std::pow(10.0f, makeupGain / 20.0f);
    compressed *= makeupLinear;

    // 7. Dry/wet mix
    float output = mix * compressed + (1.0f - mix) * input;

    // 8. Update meters
    currentGainReduction.store(20.0f * std::log10(gain));

    return output;
}
```

## Stereo Processing

For stereo operation, we use **linked** detection to prevent image shifting:

```cpp
void processBlock(AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample) {
        // Linked stereo: detect from sum of channels
        float sumLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            sumLevel += std::abs(buffer.getSample(ch, sample));
        }
        sumLevel /= numChannels;

        // Calculate gain from linked level
        float gain = calculateGain(sumLevel);

        // Apply same gain to all channels
        for (int ch = 0; ch < numChannels; ++ch) {
            float in = buffer.getSample(ch, sample);
            float out = processSampleWithGain(in, gain);
            buffer.setSample(ch, sample, out);
        }
    }
}
```

## Metering

Thread-safe metering for UI display:

```cpp
// In audio thread
currentGainReduction.store(-gainDb, std::memory_order_relaxed);
currentOutputLevel.store(outputDb, std::memory_order_relaxed);

// In UI thread
float gr = processor.getCurrentGainReduction();  // Uses atomic load
```

## Performance Considerations

1. **No allocations** in processBlock - all buffers pre-allocated
2. **Minimal branching** in inner loop
3. **SIMD potential** - envelope processing can be vectorized
4. **Atomic metering** - no locks in audio thread

## References

- [Universal Audio LA-2A Manual](https://www.uaudio.com/support/product-info/la-2a)
- [Teletronix T4 Datasheet](http://www.intactaudio.com/forum/viewtopic.php?f=6&t=5420)
- [DAFX: Digital Audio Effects (Zölzer)](https://www.dafx.de/) - Chapter on dynamics processing
