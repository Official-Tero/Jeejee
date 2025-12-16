# Framework Decision: Why JUCE

## Context

This document records the framework selection process for the LA2ATero AU plugin project.

## Requirements

- **Target Format**: Audio Units (AU) only
- **Platform**: macOS only
- **Purpose**: Personal learning project
- **Plugin Type**: Audio effect (optical compressor)

## Frameworks Evaluated

### 1. JUCE (C++)

**Pros:**
- Industry standard, widely used in commercial plugins
- Excellent AU support with automatic component registration
- Comprehensive DSP utilities and audio buffer handling
- Built-in parameter system (AudioProcessorValueTreeState)
- Cross-platform UI framework with custom LookAndFeel support
- Large community and documentation
- CMake integration

**Cons:**
- C++ complexity
- Larger binary size due to framework overhead
- Dual licensing (GPL or commercial)

### 2. nih-plug (Rust)

**Pros:**
- Modern Rust safety guarantees
- Clean API design
- Active development

**Cons:**
- **No AU format support** - only outputs CLAP and VST3
- Smaller community
- Less mature ecosystem

### 3. Apple Native Audio Unit SDK

**Pros:**
- Direct Apple API access
- No external dependencies
- Smallest binary size

**Cons:**
- Significant boilerplate code
- Manual parameter handling
- Limited UI support (need separate framework)
- Steeper learning curve for AU specifics

### 4. iPlug2 (C++)

**Pros:**
- Simpler than JUCE
- Good AU support
- Built-in graphics library

**Cons:**
- Smaller community than JUCE
- Less comprehensive documentation

### 5. FAUST (DSP Language)

**Pros:**
- DSP-focused domain language
- Generates optimized C++ code
- Good for algorithm prototyping

**Cons:**
- Limited UI capabilities
- Additional compilation step
- Less control over low-level details

## Decision

**Selected: JUCE**

### Initial Attempt: nih-plug

We initially selected nih-plug for its modern Rust approach. However, during implementation we discovered that **nih-plug does not support AU format** - it only generates CLAP and VST3 plugins.

```
# nih-plug build output - no AU!
build/
├── LA2ATero.clap
└── LA2ATero.vst3
```

This was a critical blocker since AU-only was a hard requirement.

### Final Selection: JUCE

After the nih-plug discovery, we pivoted to JUCE which provides:

1. **Native AU support** with `FORMATS AU` in CMake configuration
2. **Automatic installation** to `~/Library/Audio/Plug-Ins/Components/`
3. **auval validation** passes without issues
4. **Rich UI framework** enabling the vintage LA-2A visual style

## Lessons Learned

1. **Verify format support early** - Don't assume a framework supports all plugin formats
2. **Test the build pipeline** before significant implementation work
3. **JUCE remains the safest choice** for AU plugin development

## References

- [JUCE Framework](https://juce.com/)
- [nih-plug GitHub](https://github.com/robbert-vdh/nih-plug) - Note: CLAP/VST3 only
- [Apple Audio Unit Documentation](https://developer.apple.com/documentation/audiounit)
