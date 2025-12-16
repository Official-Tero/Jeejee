#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

/**
 * Vintage VU Meter Component
 *
 * Displays an analog-style meter with:
 * - Animated needle with proper VU ballistics (300ms integration)
 * - Cream face with dB scale
 * - Red zone for high levels
 * - Switchable between Gain Reduction and Output modes
 */
class VUMeter : public juce::Component, private juce::Timer
{
public:
    enum class Mode { GainReduction, Output };

    VUMeter();
    ~VUMeter() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLevel(float dB);
    void setMode(Mode newMode);
    Mode getMode() const { return mode; }

private:
    void timerCallback() override;

    float currentLevel = -60.0f;       // Current displayed level in dB
    float targetLevel = -60.0f;        // Target level (from audio thread)
    float needleAngle = 0.0f;          // Current needle angle

    Mode mode = Mode::GainReduction;

    // VU Ballistics
    static constexpr float INTEGRATION_TIME_MS = 300.0f;
    static constexpr float FRAME_RATE = 60.0f;
    float smoothingCoeff = 0.0f;

    // Meter range
    static constexpr float MIN_DB = -20.0f;
    static constexpr float MAX_DB = 3.0f;

    // Drawing helpers
    void drawMeterFace(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawNeedle(juce::Graphics& g, juce::Rectangle<float> bounds, float angle);
    void drawScale(juce::Graphics& g, juce::Rectangle<float> bounds);
    float levelToAngle(float dB);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
