# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Documentation

See [README.md](README.md) for project overview and quick start.

Detailed documentation is available in `/docs/`:

- **[architecture.md](docs/architecture.md)** - System architecture, component responsibilities, data flow
- **[dsp-design.md](docs/dsp-design.md)** - T4 opto-cell algorithm, compression curves, stereo linking
- **[ui-design.md](docs/ui-design.md)** - Visual design, color palette, component styling
- **[framework-decision.md](docs/framework-decision.md)** - Why JUCE was chosen over alternatives

Module-specific guidance:
- `src/dsp/CLAUDE.md` - DSP implementation details and constraints
- `src/ui/CLAUDE.md` - UI component patterns and styling

## Build Commands

```bash
# Configure (first time or after CMakeLists.txt changes)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j8
```

The AU plugin is automatically copied to `~/Library/Audio/Plug-Ins/Components/` after build.

## Validate Plugin

```bash
# List AU plugins
auval -a | grep LA2ATero

# Full validation
auval -v aufx La2t Tero
```

## Architecture

LA2ATero is an LA-2A style optical compressor AU plugin built with JUCE.

### Directory Structure

```
src/
├── PluginProcessor.h/cpp    # Audio processing, parameter management
├── PluginEditor.h/cpp       # Main UI layout (rack-mount style)
├── dsp/
│   └── OptoCompressor.h/cpp # T4 opto-cell compression algorithm
└── ui/
    ├── VUMeter.h/cpp        # Animated VU meter component
    └── LA2ALookAndFeel.h/cpp # Vintage visual styling
```

### DSP

- **OptoCompressor**: Simulates T4B electro-optical attenuator behavior
  - Program-dependent attack (10-100ms)
  - Two-stage release (60ms fast + 1-15s adaptive slow)
  - Soft-knee compression curve
  - Compress mode (3:1) / Limit mode (100:1)

### Parameters

| ID | Name | Range | Default |
|----|------|-------|---------|
| `peakReduction` | Peak Reduction | 0-100 | 0 |
| `gain` | Gain | -10 to +40 dB | 0 |
| `limitMode` | Limit/Compress | bool | false |
| `mix` | Mix | 0-100% | 100% |

### UI

- 800x250 rack-mount layout
- Silver faceplate with black rack ears
- VU meter (switchable GR/Output)
- Knob scales printed on faceplate

## Plugin Identifiers

- **Name**: LA2ATero
- **Manufacturer**: Tero
- **Manufacturer Code**: `Tero`
- **Plugin Code**: `La2t`
- **Type**: `aufx` (audio effect)

## Dependencies

JUCE framework included as git submodule in `/JUCE`.
