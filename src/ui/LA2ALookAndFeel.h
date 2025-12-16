#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * LA-2A Vintage Look and Feel
 *
 * Custom styling matching the classic Teletronix LA-2A hardware:
 * - Silver/gray metal faceplate
 * - Black bakelite knobs with white pointer
 * - Vintage toggle switches
 */
class LA2ALookAndFeel : public juce::LookAndFeel_V4
{
public:
    LA2ALookAndFeel();
    ~LA2ALookAndFeel() override = default;

    // Colors - Silver/Gray LA-2A scheme
    static inline const juce::Colour FACEPLATE = juce::Colour(0xFFB8B8B8);      // Silver-gray metal
    static inline const juce::Colour FACEPLATE_DARK = juce::Colour(0xFFA0A0A0); // Darker gray
    static inline const juce::Colour TEXT_DARK = juce::Colour(0xFF2A2A2A);      // Dark text
    static inline const juce::Colour KNOB_BLACK = juce::Colour(0xFF1A1A1A);     // Black bakelite
    static inline const juce::Colour METER_BEZEL = juce::Colour(0xFF4A5568);    // Blue-gray bezel
    static inline const juce::Colour METER_FACE = juce::Colour(0xFFF5E6C8);     // Cream/yellow meter
    static inline const juce::Colour TELETRONIX_RED = juce::Colour(0xFFC41E3A); // Red logo color
    static inline const juce::Colour RACK_EAR = juce::Colour(0xFF1A1A1A);       // Black rack ears

    // Rotary slider (black bakelite knob)
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override;

    // Toggle button (vintage switch)
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    // Label styling
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    juce::Font getLabelFont(juce::Label& label) override;

private:
    void drawBakeliteKnob(juce::Graphics& g, float centerX, float centerY,
                          float radius, float angle);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LA2ALookAndFeel)
};
