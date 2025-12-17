#include "LA2ALookAndFeel.h"

LA2ALookAndFeel::LA2ALookAndFeel()
{
    setColour(juce::Label::textColourId, TEXT_DARK);
    setColour(juce::Slider::textBoxTextColourId, TEXT_DARK);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void LA2ALookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle,
                                       float rotaryEndAngle, juce::Slider& /*slider*/)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centerX = bounds.getCentreX();
    auto centerY = bounds.getCentreY();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

    // Custom range: 7 o'clock to 5 o'clock (300 degrees)
    constexpr float startAngle = -5.0f * juce::MathConstants<float>::pi / 6.0f;  // 7 o'clock
    constexpr float endAngle = 5.0f * juce::MathConstants<float>::pi / 6.0f;     // 5 o'clock

    // Ignore the passed angles and use our custom range
    (void)rotaryStartAngle;
    (void)rotaryEndAngle;

    auto angle = startAngle + sliderPos * (endAngle - startAngle);

    drawDialKnob(g, centerX, centerY, radius, angle, sliderPos);
}

void LA2ALookAndFeel::drawDialKnob(juce::Graphics& g, float centerX, float centerY,
                                    float radius, float angle, float sliderPos)
{
    // Custom range for scale drawing
    constexpr float startAngle = -5.0f * juce::MathConstants<float>::pi / 6.0f;
    constexpr float endAngle = 5.0f * juce::MathConstants<float>::pi / 6.0f;

    // Outer black bezel
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillEllipse(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f);

    // Scale ring (cream/white background)
    float scaleRingOuter = radius - 3.0f;
    float scaleRingInner = radius * 0.65f;

    juce::Path scaleRing;
    scaleRing.addPieSegment(centerX - scaleRingOuter, centerY - scaleRingOuter,
                            scaleRingOuter * 2.0f, scaleRingOuter * 2.0f,
                            startAngle, endAngle, scaleRingInner / scaleRingOuter);

    // Cream gradient for scale background
    juce::ColourGradient scaleGradient(
        juce::Colour(0xFFFAF5E8), centerX, centerY - scaleRingOuter,
        juce::Colour(0xFFE8E0D0), centerX, centerY + scaleRingOuter, false);
    g.setGradientFill(scaleGradient);
    g.fillPath(scaleRing);

    // Draw scale tick marks
    g.setColour(juce::Colour(0xFF1A1A1A));
    int numTicks = 11;  // 0-10 scale
    for (int i = 0; i <= numTicks; ++i)
    {
        float tickPos = static_cast<float>(i) / static_cast<float>(numTicks);
        float tickAngle = startAngle + tickPos * (endAngle - startAngle);

        bool isMajor = (i % 2 == 0);  // Major ticks at 0, 2, 4, 6, 8, 10
        float tickInner = isMajor ? (scaleRingInner + 2.0f) : (scaleRingOuter - 8.0f);
        float tickOuter = scaleRingOuter - 2.0f;

        float x1 = centerX + tickInner * std::cos(tickAngle - juce::MathConstants<float>::halfPi);
        float y1 = centerY + tickInner * std::sin(tickAngle - juce::MathConstants<float>::halfPi);
        float x2 = centerX + tickOuter * std::cos(tickAngle - juce::MathConstants<float>::halfPi);
        float y2 = centerY + tickOuter * std::sin(tickAngle - juce::MathConstants<float>::halfPi);

        g.drawLine(x1, y1, x2, y2, isMajor ? 1.5f : 1.0f);
    }

    // Center knob (dark bakelite style)
    float knobRadius = scaleRingInner - 4.0f;

    // Knob shadow
    g.setColour(juce::Colour(0x40000000));
    g.fillEllipse(centerX - knobRadius + 2.0f, centerY - knobRadius + 2.0f,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // Knob body gradient
    juce::ColourGradient knobGradient(
        juce::Colour(0xFF4A4A4A), centerX - knobRadius * 0.3f, centerY - knobRadius * 0.3f,
        juce::Colour(0xFF1A1A1A), centerX + knobRadius * 0.5f, centerY + knobRadius * 0.5f, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centerX - knobRadius, centerY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // Knob highlight rim
    g.setColour(juce::Colour(0x20FFFFFF));
    g.drawEllipse(centerX - knobRadius + 1.0f, centerY - knobRadius + 1.0f,
                  knobRadius * 2.0f - 2.0f, knobRadius * 2.0f - 2.0f, 1.5f);

    // Red pointer/indicator line
    float pointerLength = scaleRingOuter - 4.0f;
    float pointerInner = knobRadius * 0.3f;

    float px1 = centerX + pointerInner * std::cos(angle - juce::MathConstants<float>::halfPi);
    float py1 = centerY + pointerInner * std::sin(angle - juce::MathConstants<float>::halfPi);
    float px2 = centerX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    float py2 = centerY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);

    g.setColour(juce::Colour(0xFFCC0000));
    g.drawLine(px1, py1, px2, py2, 2.5f);

    // Small center cap
    float capRadius = knobRadius * 0.25f;
    g.setColour(juce::Colour(0xFF0A0A0A));
    g.fillEllipse(centerX - capRadius, centerY - capRadius,
                  capRadius * 2.0f, capRadius * 2.0f);
}

void LA2ALookAndFeel::drawBakeliteKnob(juce::Graphics& g, float centerX, float centerY,
                                        float radius, float angle)
{
    // Legacy function - redirect to new dial style
    drawDialKnob(g, centerX, centerY, radius, angle, 0.5f);
}

void LA2ALookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                       juce::Slider::SliderStyle style, juce::Slider& /*slider*/)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearVertical)
        return;

    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));

    bool isHorizontal = (style == juce::Slider::LinearHorizontal);

    if (isHorizontal)
    {
        float trackHeight = 6.0f;
        float trackY = bounds.getCentreY() - trackHeight / 2.0f;

        // Track background - dark
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, 3.0f);

        // Track fill - green to yellow to red gradient (like Dorrough meter)
        float fillWidth = sliderPos - bounds.getX();
        if (fillWidth > 0)
        {
            // Create multi-stop gradient: green -> yellow -> red
            juce::ColourGradient trackGradient(
                juce::Colour(0xFF00DD00), bounds.getX(), trackY,
                juce::Colour(0xFFFF3300), bounds.getX() + bounds.getWidth(), trackY, false);
            // Add yellow in the middle
            trackGradient.addColour(0.5, juce::Colour(0xFFFFAA00));
            g.setGradientFill(trackGradient);
            g.fillRoundedRectangle(bounds.getX(), trackY, fillWidth, trackHeight, 3.0f);
        }

        // Fader thumb - black with gradient
        float thumbWidth = 14.0f;
        float thumbHeight = height * 0.9f;
        float thumbX = sliderPos - thumbWidth / 2.0f;
        float thumbY = bounds.getCentreY() - thumbHeight / 2.0f;

        // Thumb shadow
        g.setColour(juce::Colour(0x50000000));
        g.fillRoundedRectangle(thumbX + 2.0f, thumbY + 2.0f, thumbWidth, thumbHeight, 3.0f);

        // Thumb body
        juce::ColourGradient thumbGradient(
            juce::Colour(0xFF3A3A3A), thumbX, thumbY,
            juce::Colour(0xFF1A1A1A), thumbX + thumbWidth, thumbY + thumbHeight, false);
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 3.0f);

        // Thumb highlight
        g.setColour(juce::Colour(0x30FFFFFF));
        g.fillRoundedRectangle(thumbX + 2.0f, thumbY + 2.0f, thumbWidth - 4.0f, thumbHeight * 0.3f, 2.0f);

        // Thumb center line
        g.setColour(juce::Colour(0xFF666666));
        g.drawLine(thumbX + thumbWidth / 2.0f, thumbY + 4.0f,
                   thumbX + thumbWidth / 2.0f, thumbY + thumbHeight - 4.0f, 1.5f);
    }
}

void LA2ALookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    auto isOn = button.getToggleState();

    // Button shadow
    g.setColour(juce::Colour(0x50000000));
    g.fillRoundedRectangle(bounds.translated(2.0f, 2.0f), 4.0f);

    // Button background - dark when off
    if (isOn)
    {
        // Illuminated with Dorrough meter gradient (green -> yellow -> red)
        juce::ColourGradient glowGradient(
            juce::Colour(0xFF00DD00), bounds.getX(), bounds.getCentreY(),
            juce::Colour(0xFFFF3300), bounds.getRight(), bounds.getCentreY(), false);
        glowGradient.addColour(0.5, juce::Colour(0xFFFFAA00));
        g.setGradientFill(glowGradient);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Glow effect
        g.setColour(juce::Colour(0x40FFFF00));
        g.drawRoundedRectangle(bounds.expanded(2.0f), 5.0f, 3.0f);

        // Inner highlight
        g.setColour(juce::Colour(0x40FFFFFF));
        g.fillRoundedRectangle(bounds.reduced(2.0f).withHeight(bounds.getHeight() * 0.4f), 3.0f);
    }
    else
    {
        // Dark/off state
        juce::ColourGradient offGradient(
            juce::Colour(0xFF3A3A3A), bounds.getX(), bounds.getY(),
            juce::Colour(0xFF1A1A1A), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(offGradient);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Dim indicator colors
        juce::ColourGradient dimGradient(
            juce::Colour(0xFF0A3A0A), bounds.getX(), bounds.getCentreY(),
            juce::Colour(0xFF3A0A0A), bounds.getRight(), bounds.getCentreY(), false);
        dimGradient.addColour(0.5, juce::Colour(0xFF3A2A0A));
        g.setGradientFill(dimGradient);
        g.fillRoundedRectangle(bounds.reduced(3.0f), 3.0f);
    }

    // Border
    g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(0xFF666666) : juce::Colour(0xFF444444));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

    // Button text
    g.setColour(isOn ? juce::Colour(0xFF1A1A1A) : juce::Colour(0xFF888888));
    g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    g.drawText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred);
}

void LA2ALookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(TEXT_DARK);
    g.setFont(getLabelFont(label));

    auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
    g.drawText(label.getText(), textArea, label.getJustificationType(), true);
}

juce::Font LA2ALookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(label.getFont().getHeight(), juce::Font::bold);
}
