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
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    drawBakeliteKnob(g, centerX, centerY, radius, angle);
}

void LA2ALookAndFeel::drawBakeliteKnob(juce::Graphics& g, float centerX, float centerY,
                                        float radius, float angle)
{
    // Knob shadow
    g.setColour(juce::Colour(0x60000000));
    g.fillEllipse(centerX - radius + 4.0f, centerY - radius + 4.0f,
                  radius * 2.0f, radius * 2.0f);

    // Main knob body - black bakelite with subtle gradient
    juce::ColourGradient knobGradient(
        juce::Colour(0xFF3A3A3A), centerX - radius * 0.3f, centerY - radius * 0.3f,
        juce::Colour(0xFF151515), centerX + radius * 0.5f, centerY + radius * 0.5f, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f);

    // Knob rim highlight
    g.setColour(juce::Colour(0x25FFFFFF));
    g.drawEllipse(centerX - radius + 1.0f, centerY - radius + 1.0f,
                  radius * 2.0f - 2.0f, radius * 2.0f - 2.0f, 2.0f);

    // White pointer line
    juce::Path pointer;
    auto pointerLength = radius * 0.75f;
    auto pointerWidth = 3.0f;

    pointer.addRoundedRectangle(-pointerWidth / 2.0f, -radius + 4.0f,
                                 pointerWidth, pointerLength, 1.5f);

    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));

    // Pointer
    g.setColour(juce::Colour(0xFFEEEEEE));
    g.fillPath(pointer);

    g.restoreState();

    // Center indent
    auto indentRadius = radius * 0.15f;
    g.setColour(juce::Colour(0xFF0A0A0A));
    g.fillEllipse(centerX - indentRadius, centerY - indentRadius,
                  indentRadius * 2.0f, indentRadius * 2.0f);
}

void LA2ALookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                       bool /*shouldDrawButtonAsHighlighted*/,
                                       bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto isOn = button.getToggleState();

    // Hex nut base
    auto nutSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.7f;
    auto nutCenterX = bounds.getCentreX();
    auto nutCenterY = bounds.getCentreY() - 5.0f;

    // Draw hex nut shape
    juce::Path hexNut;
    for (int i = 0; i < 6; ++i)
    {
        float angle = juce::MathConstants<float>::pi / 6.0f + i * juce::MathConstants<float>::pi / 3.0f;
        float px = nutCenterX + nutSize * 0.5f * std::cos(angle);
        float py = nutCenterY + nutSize * 0.5f * std::sin(angle);
        if (i == 0)
            hexNut.startNewSubPath(px, py);
        else
            hexNut.lineTo(px, py);
    }
    hexNut.closeSubPath();

    // Hex nut shadow
    g.setColour(juce::Colour(0x40000000));
    g.fillPath(hexNut, juce::AffineTransform::translation(2.0f, 2.0f));

    // Hex nut body
    juce::ColourGradient nutGradient(
        juce::Colour(0xFFAAAAAA), nutCenterX - nutSize * 0.3f, nutCenterY - nutSize * 0.3f,
        juce::Colour(0xFF666666), nutCenterX + nutSize * 0.3f, nutCenterY + nutSize * 0.3f, true);
    g.setGradientFill(nutGradient);
    g.fillPath(hexNut);

    // Toggle lever
    auto leverLength = nutSize * 0.8f;
    auto leverWidth = nutSize * 0.25f;
    float leverAngle = isOn ? -juce::MathConstants<float>::pi * 0.25f
                            : juce::MathConstants<float>::pi * 0.25f;

    juce::Path lever;
    lever.addRoundedRectangle(-leverWidth / 2.0f, -leverLength * 0.8f,
                               leverWidth, leverLength, 3.0f);

    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(leverAngle).translated(nutCenterX, nutCenterY));

    // Lever shadow
    g.setColour(juce::Colour(0x40000000));
    g.fillPath(lever, juce::AffineTransform::translation(2.0f, 2.0f));

    // Lever body
    juce::ColourGradient leverGradient(
        juce::Colour(0xFFCCCCCC), -leverWidth / 2.0f, 0.0f,
        juce::Colour(0xFF888888), leverWidth / 2.0f, 0.0f, false);
    g.setGradientFill(leverGradient);
    g.fillPath(lever);

    g.restoreState();

    // Labels
    g.setColour(TEXT_DARK);
    g.setFont(9.0f);
    g.drawText("LIMIT", static_cast<int>(bounds.getX()),
               static_cast<int>(nutCenterY - nutSize * 0.5f - 16),
               static_cast<int>(bounds.getWidth()), 14, juce::Justification::centred);
    g.drawText("COMP", static_cast<int>(bounds.getX()),
               static_cast<int>(nutCenterY + nutSize * 0.5f + 4),
               static_cast<int>(bounds.getWidth()), 14, juce::Justification::centred);
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
