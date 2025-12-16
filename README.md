# LA2ATero

An LA-2A style optical compressor Audio Unit plugin for macOS, built with JUCE.

## Features

- **T4 Opto-Cell Modeling**: Authentic program-dependent attack/release behavior
- **Two Modes**: Compress (3:1) and Limit (100:1)
- **Analog VU Meter**: Switchable between Gain Reduction and Output
- **Parallel Compression**: Dry/wet mix control
- **Vintage UI**: Rack-mount design matching original LA-2A hardware

## Requirements

- macOS 10.13+
- CMake 3.15+
- Xcode Command Line Tools

## Building

```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/au-demo.git
cd au-demo

# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j8
```

The plugin is automatically installed to `~/Library/Audio/Plug-Ins/Components/`.

## Validation

```bash
# Verify installation
auval -a | grep LA2ATero

# Full validation
auval -v aufx La2t Tero
```

## Controls

| Control | Range | Description |
|---------|-------|-------------|
| Peak Reduction | 0-100 | Amount of compression (threshold) |
| Gain | -10 to +40 dB | Makeup gain |
| Limit/Compress | Switch | Compression ratio mode |
| Mix | 0-100% | Dry/wet blend |
| GR/OUT | Button | VU meter mode |

## Documentation

- [Architecture](docs/architecture.md) - System design and component overview
- [DSP Design](docs/dsp-design.md) - T4 opto-cell algorithm details
- [UI Design](docs/ui-design.md) - Visual specifications and styling
- [Framework Decision](docs/framework-decision.md) - Why JUCE was chosen

## Project Structure

```
├── src/
│   ├── PluginProcessor.cpp   # Audio processing and parameters
│   ├── PluginEditor.cpp      # Main UI layout
│   ├── dsp/
│   │   └── OptoCompressor.cpp # Compression algorithm
│   └── ui/
│       ├── VUMeter.cpp       # Animated meter
│       └── LA2ALookAndFeel.cpp # Vintage styling
├── docs/                      # Documentation
├── JUCE/                      # Framework (submodule)
└── CMakeLists.txt            # Build configuration
```

## License

Personal/educational project.
