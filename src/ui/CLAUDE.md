# UI Module - Claude Code Guidance

## Overview

This module contains the visual components for the LA2ATero plugin, styled after the Teletronix LA-2A hardware compressor.

## Files

- `VUMeter.h/cpp` - Animated analog-style VU meter
- `LA2ALookAndFeel.h/cpp` - Custom JUCE LookAndFeel with vintage styling

## Visual Design

### Hardware Reference

The LA-2A is a 2U rack-mount unit with:
- Silver/gray brushed aluminum faceplate
- Black rack ears with mounting slots
- Large center VU meter with blue-gray bezel
- Black bakelite chicken-head knobs
- Red "TELETRONIX" branding

### Color Constants (in LA2ALookAndFeel.h)

```cpp
FACEPLATE       = 0xFFB8B8B8  // Silver-gray
FACEPLATE_DARK  = 0xFF8A8A8A  // Gradient shadow
METER_BEZEL     = 0xFF4A5568  // Blue-gray
METER_FACE      = 0xFFF5E6C8  // Cream/yellow
TELETRONIX_RED  = 0xFFC41E3A  // Logo red
RACK_EAR        = 0xFF1A1A1A  // Black
KNOB_COLOR      = 0xFF1A1A1A  // Black bakelite
```

## VUMeter Component

### Ballistics

Standard VU meter behavior:
- **Integration time**: 300ms
- **Update rate**: 30fps (via timer in PluginEditor)
- **Overshoot**: ~1%

### Modes

- **GR (Gain Reduction)**: Shows compression amount (0 to -20 dB)
- **OUT (Output)**: Shows output level

### Needle Animation

The needle uses smoothed animation to simulate mechanical inertia:

```cpp
displayLevel += 0.1f * (targetLevel - displayLevel);
```

## LA2ALookAndFeel

Custom styling for JUCE components.

### Knob Style

Black bakelite with white pointer line:
- Circular body with subtle edge highlight
- White indicator line from center toward edge
- Rotation range: -140° to +140°

### Toggle Switch

Hex-nut style toggle for LIMIT/COMPRESS:
- Vertical orientation
- Silver lever on dark plate
- Labels above and below

## Layout Guidelines

### Main Window: 800 x 250 pixels

```
[Rack Ear 30px][        Content Area 740px        ][Rack Ear 30px]
```

### Component Positioning

Knob scales are drawn on the faceplate (in PluginEditor::paint), not in the LookAndFeel. This allows the scale to remain static while the knob rotates.

## Thread Safety

UI components only run on the message thread. Metering values are read from atomics:

```cpp
// In timerCallback()
float gr = processorRef.getCurrentGainReduction();
vuMeter.setLevel(gr);
```

## Extending the UI

When adding new controls:

1. Add component member to `PluginEditor`
2. Add parameter attachment if needed
3. Position in `resized()`
4. Style in `LA2ALookAndFeel` if custom appearance needed
5. Update knob scales in `paint()` if applicable

## References

See `/docs/ui-design.md` for detailed visual specifications.
