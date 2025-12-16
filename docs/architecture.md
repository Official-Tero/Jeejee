# LA2ATero Architecture

## Overview

LA2ATero is an LA-2A style optical compressor implemented as a macOS Audio Unit (AU) plugin using the JUCE framework.

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Host DAW                                │
│  (Logic Pro, Ableton, GarageBand, etc.)                        │
└─────────────────────────────────┬───────────────────────────────┘
                                  │ Audio Units API
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                    LA2ATero.component                           │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                   PluginProcessor                         │  │
│  │  - Parameter management (APVTS)                          │  │
│  │  - Audio callback routing                                │  │
│  │  - State save/restore                                    │  │
│  │                         │                                │  │
│  │                         ▼                                │  │
│  │  ┌─────────────────────────────────────────────────┐    │  │
│  │  │              OptoCompressor (DSP)                │    │  │
│  │  │  - T4 opto-cell simulation                      │    │  │
│  │  │  - Gain reduction calculation                   │    │  │
│  │  │  - Metering output                              │    │  │
│  │  └─────────────────────────────────────────────────┘    │  │
│  └──────────────────────────────────────────────────────────┘  │
│                              │                                  │
│                              │ Atomic metering values           │
│                              ▼                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    PluginEditor                           │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │  │
│  │  │  VUMeter    │  │   Knobs     │  │ LA2ALookAndFeel │   │  │
│  │  │ (animated)  │  │  (sliders)  │  │   (styling)     │   │  │
│  │  └─────────────┘  └─────────────┘  └─────────────────────┘   │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
au-demo/
├── CMakeLists.txt              # Build configuration
├── CLAUDE.md                   # Claude Code guidance
├── JUCE/                       # JUCE framework (submodule)
├── docs/                       # Project documentation
│   ├── architecture.md         # This file
│   ├── dsp-design.md          # DSP implementation details
│   ├── ui-design.md           # UI design decisions
│   └── framework-decision.md  # Framework selection rationale
├── src/
│   ├── PluginProcessor.h/cpp  # Main audio processor
│   ├── PluginEditor.h/cpp     # Main UI component
│   ├── dsp/
│   │   ├── CLAUDE.md          # DSP-specific guidance
│   │   └── OptoCompressor.h/cpp
│   └── ui/
│       ├── CLAUDE.md          # UI-specific guidance
│       ├── VUMeter.h/cpp
│       └── LA2ALookAndFeel.h/cpp
└── build/                      # Build output (gitignored)
```

## Component Responsibilities

### PluginProcessor

The central coordinator that:

1. **Defines parameters** via `AudioProcessorValueTreeState`
2. **Routes audio** through the OptoCompressor in `processBlock()`
3. **Exposes metering** data to the editor via thread-safe getters
4. **Handles state** serialization for preset save/load

Key interfaces:
```cpp
class AuDemoProcessor : public juce::AudioProcessorValueTreeState::Listener {
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&);

    float getCurrentGainReduction() const;
    float getCurrentOutputLevel() const;
};
```

### OptoCompressor

The DSP engine that models T4B optical attenuator behavior:

1. **Level detection** with optical cell response characteristics
2. **Gain computation** with program-dependent attack/release
3. **Two-stage release** envelope (fast + slow)
4. **Compression curve** with soft knee

See [dsp-design.md](dsp-design.md) for detailed algorithm documentation.

### PluginEditor

The UI component that:

1. **Renders** the rack-mount visual design
2. **Binds controls** to parameters via attachments
3. **Animates** the VU meter at 30fps
4. **Applies** LA2ALookAndFeel for vintage styling

### VUMeter

A custom component implementing:

1. **Analog ballistics** (300ms integration time)
2. **Needle animation** with inertia
3. **Dual mode** display (GR or Output)
4. **Vintage appearance** with proper scale markings

### LA2ALookAndFeel

Custom JUCE LookAndFeel providing:

1. **Color palette** matching LA-2A hardware
2. **Knob rendering** (black bakelite style)
3. **Toggle switch** appearance
4. **Typography** for vintage labels

See [ui-design.md](ui-design.md) for visual design details.

## Data Flow

### Audio Processing (Real-time Thread)

```
Host Audio Buffer
       │
       ▼
┌──────────────────┐
│ processBlock()   │
│                  │
│  for each sample:│
│    ┌─────────────┴──────────────┐
│    │ OptoCompressor.process()   │
│    │  1. Detect input level     │
│    │  2. Compute gain reduction │
│    │  3. Apply compression      │
│    │  4. Apply makeup gain      │
│    │  5. Mix dry/wet            │
│    │  6. Update meters (atomic) │
│    └─────────────┬──────────────┘
│                  │
└──────────────────┘
       │
       ▼
Processed Audio Buffer
```

### Parameter Updates

```
UI Control Change
       │
       ▼
APVTS Attachment
       │
       ▼
Parameter Smoothing (automatic)
       │
       ▼
DSP reads smoothed value in processBlock()
```

### Metering (Thread-safe)

```
Audio Thread                    UI Thread
     │                              │
     │ atomic<float> store          │
     └──────────────────────────────┤
                                    │ atomic<float> load
                                    ▼
                              VUMeter.repaint()
```

## Thread Safety

| Data | Access Pattern | Synchronization |
|------|----------------|-----------------|
| Audio buffers | Audio thread only | None needed |
| Parameters | Both threads | APVTS + SmoothedValue |
| Meter levels | Audio writes, UI reads | std::atomic<float> |
| UI state | UI thread only | None needed |

## Build System

CMake configuration with JUCE:

```cmake
juce_add_plugin(AuDemo
    FORMATS AU                    # AU only
    PLUGIN_CODE La2t             # 4-char identifier
    PLUGIN_MANUFACTURER_CODE Tero
    PRODUCT_NAME "LA2ATero"
)

target_sources(AuDemo PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/dsp/OptoCompressor.cpp
    src/ui/VUMeter.cpp
    src/ui/LA2ALookAndFeel.cpp
)
```

## Plugin Installation

After build, the component is automatically copied to:
```
~/Library/Audio/Plug-Ins/Components/LA2ATero.component
```

Validation:
```bash
auval -v aufx La2t Tero
```
