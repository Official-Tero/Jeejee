# UI Design: LA-2A Visual Recreation

## Overview

The LA2ATero UI recreates the visual appearance of the Teletronix LA-2A hardware compressor in a digital plugin format.

## Hardware Reference

The LA-2A is a 2U rack-mount unit with these visual characteristics:

- **Faceplate**: Brushed silver/gray aluminum
- **Rack ears**: Black painted steel with mounting slots
- **VU Meter**: Large center-mounted meter with blue-gray bezel
- **Knobs**: Black bakelite chicken-head style
- **Labels**: Silk-screened text, red "TELETRONIX" branding
- **Switch**: Toggle switch for LIMIT/COMPRESS selection

## Color Palette

```cpp
// From LA2ALookAndFeel.h
static inline const juce::Colour FACEPLATE      = juce::Colour(0xFFB8B8B8);  // Silver-gray
static inline const juce::Colour FACEPLATE_DARK = juce::Colour(0xFF8A8A8A);  // Gradient shadow
static inline const juce::Colour METER_BEZEL    = juce::Colour(0xFF4A5568);  // Blue-gray
static inline const juce::Colour METER_FACE     = juce::Colour(0xFFF5E6C8);  // Cream/yellow
static inline const juce::Colour NEEDLE_COLOR   = juce::Colour(0xFF1A1A1A);  // Black
static inline const juce::Colour TEXT_DARK      = juce::Colour(0xFF2D2D2D);  // Dark labels
static inline const juce::Colour TELETRONIX_RED = juce::Colour(0xFFC41E3A);  // Red logo
static inline const juce::Colour RACK_EAR       = juce::Colour(0xFF1A1A1A);  // Black
static inline const juce::Colour KNOB_COLOR     = juce::Colour(0xFF1A1A1A);  // Black bakelite
```

## Layout

### Dimensions

- **Window size**: 800 x 250 pixels (rack-mount proportions, ~3.2:1 ratio)
- **Rack ears**: 30 pixels wide on each side
- **VU Meter**: Central position, 180 x 120 pixels

### Layout Structure

```
┌──────┬─────────────────────────────────────────────────────────────┬──────┐
│      │                                                             │      │
│      │     TELETRONIX                                              │      │
│ RACK │     LA-2A LEVELING AMPLIFIER                               │ RACK │
│ EAR  │                                                             │ EAR  │
│      │            ┌─────────────────────┐                          │      │
│  ○   │  LIMIT     │                     │                          │  ○   │
│      │  ⬤────     │    VU METER         │                          │      │
│  ○   │  COMP      │                     │     PEAK                 │  ○   │
│      │            └─────────────────────┘     REDUCTION            │      │
│      │                 [GR] [OUT]                                  │      │
│      │                                                             │      │
│      │    GAIN                              ◉         MIX          │      │
│      │     ◉                                           ◉           │      │
│      │                                                             │      │
└──────┴─────────────────────────────────────────────────────────────┴──────┘
```

### Component Positions (approximate)

| Component | X | Y | Width | Height |
|-----------|---|---|-------|--------|
| Left rack ear | 0 | 0 | 30 | 250 |
| Right rack ear | 770 | 0 | 30 | 250 |
| Limit/Compress switch | 80 | 100 | 60 | 80 |
| Gain knob | 180 | 150 | 80 | 80 |
| VU Meter | 310 | 50 | 180 | 120 |
| Peak Reduction knob | 530 | 100 | 80 | 80 |
| Mix knob | 680 | 150 | 80 | 80 |

## Components

### Rack Ears

Black panels on each side with mounting slot cutouts:

```cpp
void drawRackEar(Graphics& g, Rectangle<int> bounds) {
    g.setColour(RACK_EAR);
    g.fillRect(bounds);

    // Mounting slots
    g.setColour(Colours::black);
    int slotWidth = 10;
    int slotHeight = 20;
    int slotX = bounds.getCentreX() - slotWidth / 2;

    // Top and bottom slots
    g.fillRoundedRectangle(slotX, 20, slotWidth, slotHeight, 2);
    g.fillRoundedRectangle(slotX, bounds.getHeight() - 40, slotWidth, slotHeight, 2);
}
```

### VU Meter

Animated analog-style meter with:

1. **Blue-gray bezel** with subtle gradient
2. **Cream/yellow face** with scale markings
3. **Scale**: -20 to +3 VU with red zone above 0
4. **Needle**: Black with pivot point at bottom center
5. **TELETRONIX branding** inside meter face

#### Meter Ballistics

```cpp
// VU meter standard: 300ms integration time
float integrationTime = 0.3f;  // seconds
float coefficient = 1.0f - std::exp(-1.0f / (integrationTime * refreshRate));

// In timer callback (30fps)
displayLevel += coefficient * (targetLevel - displayLevel);
```

#### Needle Animation

```cpp
void drawNeedle(Graphics& g, float level, Rectangle<float> meterBounds) {
    // Map level to angle (-45° to +45° range)
    float angle = juce::jmap(level, -20.0f, 3.0f, -45.0f, 45.0f);
    angle = juce::degreesToRadians(angle);

    // Pivot point at bottom center
    Point<float> pivot(meterBounds.getCentreX(), meterBounds.getBottom() - 10);
    float needleLength = meterBounds.getHeight() * 0.8f;

    // Calculate needle endpoint
    Point<float> needleEnd(
        pivot.x + needleLength * std::sin(angle),
        pivot.y - needleLength * std::cos(angle)
    );

    // Draw needle
    g.setColour(NEEDLE_COLOR);
    g.drawLine(pivot.x, pivot.y, needleEnd.x, needleEnd.y, 2.0f);

    // Draw pivot cap
    g.fillEllipse(pivot.x - 4, pivot.y - 4, 8, 8);
}
```

### Knobs

Black bakelite chicken-head style knobs:

```cpp
void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                      float sliderPos, float startAngle, float endAngle,
                      Slider& slider) {
    float radius = jmin(width, height) / 2.0f - 4.0f;
    float centreX = x + width / 2.0f;
    float centreY = y + height / 2.0f;
    float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Knob body - black bakelite
    g.setColour(KNOB_COLOR);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

    // Subtle edge highlight
    g.setColour(Colours::white.withAlpha(0.1f));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2, 1.0f);

    // White pointer line
    Path pointer;
    float pointerLength = radius * 0.7f;
    float pointerWidth = 3.0f;
    pointer.addRoundedRectangle(-pointerWidth / 2, -radius + 4, pointerWidth, pointerLength, 1.5f);

    g.setColour(Colours::white);
    g.fillPath(pointer, AffineTransform::rotation(angle).translated(centreX, centreY));
}
```

### Knob Scales

Scale markings are drawn on the faceplate (not on the knob):

```cpp
void drawKnobScale(Graphics& g, Point<float> center, float radius,
                   int minVal, int maxVal) {
    g.setColour(TEXT_DARK);
    g.setFont(10.0f);

    int numMarks = maxVal - minVal;
    float startAngle = -140.0f;
    float endAngle = 140.0f;
    float angleRange = endAngle - startAngle;

    for (int i = 0; i <= numMarks; i += 2) {  // Every other value
        float normalised = (float)i / numMarks;
        float angle = degreesToRadians(startAngle + normalised * angleRange);

        // Tick mark
        float innerRadius = radius + 5;
        float outerRadius = radius + 12;
        float x1 = center.x + innerRadius * std::sin(angle);
        float y1 = center.y - innerRadius * std::cos(angle);
        float x2 = center.x + outerRadius * std::sin(angle);
        float y2 = center.y - outerRadius * std::cos(angle);
        g.drawLine(x1, y1, x2, y2, 1.5f);

        // Number label
        float labelRadius = radius + 22;
        float labelX = center.x + labelRadius * std::sin(angle);
        float labelY = center.y - labelRadius * std::cos(angle);
        g.drawText(String(minVal + i), labelX - 10, labelY - 6, 20, 12, Justification::centred);
    }
}
```

### Limit/Compress Switch

Toggle switch with hex-nut styling:

```cpp
void drawToggleButton(Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(4);

    // Switch plate
    g.setColour(FACEPLATE_DARK);
    g.fillRoundedRectangle(bounds, 3.0f);

    // Switch lever
    bool isOn = button.getToggleState();
    float leverY = isOn ? bounds.getY() + 5 : bounds.getBottom() - 15;

    g.setColour(Colours::silver);
    g.fillRoundedRectangle(bounds.getX() + 5, leverY, bounds.getWidth() - 10, 10, 2);

    // Hex nut detail
    g.setColour(Colours::grey);
    float nutSize = 8;
    g.drawEllipse(bounds.getCentreX() - nutSize/2, leverY + 1, nutSize, nutSize, 1);
}
```

### Labels

Vintage typography using sans-serif fonts:

```cpp
// Title
g.setColour(TELETRONIX_RED);
g.setFont(Font(24.0f, Font::bold));
g.drawText("TELETRONIX", titleBounds, Justification::centred);

// Subtitle
g.setColour(TEXT_DARK);
g.setFont(Font(12.0f));
g.drawText("LA-2A LEVELING AMPLIFIER", subtitleBounds, Justification::centred);

// Control labels
g.setFont(Font(10.0f, Font::bold));
g.drawText("GAIN", gainLabelBounds, Justification::centred);
g.drawText("PEAK REDUCTION", peakLabelBounds, Justification::centred);
```

## Faceplate Background

Silver gradient with subtle texture:

```cpp
void paintFaceplate(Graphics& g, Rectangle<int> bounds) {
    // Vertical gradient for brushed metal effect
    ColourGradient gradient(FACEPLATE, 0, 0, FACEPLATE_DARK, 0, bounds.getHeight(), false);
    g.setGradientFill(gradient);
    g.fillRect(bounds);

    // Subtle noise texture (optional)
    Random random;
    g.setColour(Colours::white.withAlpha(0.02f));
    for (int i = 0; i < 1000; ++i) {
        int x = random.nextInt(bounds.getWidth());
        int y = random.nextInt(bounds.getHeight());
        g.fillRect(x, y, 1, 1);
    }
}
```

## Animation

### Timer-Based Updates

```cpp
// In PluginEditor
void timerCallback() override {
    // Update VU meter at 30fps
    vuMeter.setLevel(processorRef.getCurrentGainReduction());
    vuMeter.repaint();
}

// Start timer in constructor
startTimerHz(30);
```

### Smooth Transitions

All value changes use smoothing to prevent jumps:

```cpp
// Meter smoothing
currentDisplayLevel += 0.1f * (targetLevel - currentDisplayLevel);

// Needle damping (simulates mechanical inertia)
needleVelocity *= 0.9f;  // Damping
needleVelocity += 0.1f * (targetAngle - currentAngle);
currentAngle += needleVelocity;
```

## Responsive Considerations

While the current design is fixed-size (800x250), future improvements could include:

1. **Scalable graphics** using relative positioning
2. **HiDPI support** with 2x assets
3. **Resizable window** with aspect ratio lock

## Accessibility

- **High contrast** between controls and background
- **Clear labeling** for all controls
- **Sufficient size** for clickable targets (minimum 44px)
