# DSP Module - Claude Code Guidance

## Overview

This module contains the digital signal processing code for the LA2ATero optical compressor.

## Files

- `OptoCompressor.h/cpp` - T4 opto-cell compressor implementation

## Key Concepts

### T4 Opto-Cell Model

The LA-2A uses a Teletronix T4B electro-optical attenuator. Key behaviors:

1. **Program-dependent attack**: Attack time varies with signal dynamics (10-100ms)
2. **Two-stage release**: Fast initial decay (~60ms) + slow tail (1-15s adaptive)
3. **Soft-knee compression**: Gradual ratio increase as signal exceeds threshold
4. **Optical inertia**: The photocell has inherent capacitance creating smooth response

### Processing Pipeline

```
Input → Level Detection → Envelope Following → Gain Computation → Output
             ↓                    ↓
        (RMS-like)        (program-dependent
                           attack/release)
```

## Critical Requirements

### Thread Safety

Audio processing runs on a real-time thread. Requirements:

- **No allocations** in `processSample()` or `processBlock()`
- **No locks** - use `std::atomic` for metering values
- **Pre-allocated buffers** only

### Metering

```cpp
// Write in audio thread (relaxed ordering is fine)
currentGainReduction.store(value, std::memory_order_relaxed);

// Read in UI thread
float gr = currentGainReduction.load(std::memory_order_relaxed);
```

### Stereo Linking

For stereo operation, detect level from sum of channels to prevent stereo image shifting:

```cpp
float sumLevel = (std::abs(left) + std::abs(right)) / 2.0f;
float gain = computeGain(sumLevel);
// Apply same gain to both channels
```

## Parameters

| Parameter | Range | Effect |
|-----------|-------|--------|
| `peakReduction` | 0-100 | Maps to threshold (0→0dB, 100→-40dB) |
| `makeupGain` | -10 to +40 dB | Output level compensation |
| `limitMode` | bool | false=3:1, true=100:1 ratio |
| `mix` | 0-1 | Dry/wet parallel compression |

## Performance Notes

- Inner loop should be branchless where possible
- Use `std::exp` approximations if needed for performance
- Current implementation targets ~1-2% CPU on modern hardware

## Testing

Validate behavior with:

1. **Sine wave** - steady-state compression ratio
2. **Drum hits** - transient response and release
3. **Program material** - overall character
4. **Null test** - mix at 0% should pass audio unchanged

## References

See `/docs/dsp-design.md` for detailed algorithm documentation.
