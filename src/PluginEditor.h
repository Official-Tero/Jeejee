#pragma once

#include "PluginProcessor.h"
#include "ui/VUMeter.h"
#include "ui/LA2ALookAndFeel.h"

class AuDemoEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AuDemoEditor(AuDemoProcessor&);
    ~AuDemoEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawKnobScale(juce::Graphics& g, juce::Point<float> center,
                       float radius, int minVal, int maxVal);

    AuDemoProcessor& processorRef;

    // Look and Feel
    LA2ALookAndFeel la2aLookAndFeel;

    // VU Meter
    VUMeter vuMeter;
    juce::TextButton meterModeButton{"GR"};

    // Knobs
    juce::Slider peakReductionSlider;
    juce::Slider gainSlider;
    juce::Slider mixSlider;

    // Labels
    juce::Label peakReductionLabel;
    juce::Label gainLabel;
    juce::Label mixLabel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    // Switch
    juce::ToggleButton limitModeSwitch;

    // Knob scale positions (for drawing)
    juce::Point<float> gainKnobCenter;
    float gainKnobRadius = 0.0f;
    juce::Point<float> peakReductionKnobCenter;
    float peakReductionKnobRadius = 0.0f;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakReductionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limitModeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuDemoEditor)
};
