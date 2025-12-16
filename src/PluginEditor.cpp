#include "PluginEditor.h"

AuDemoEditor::AuDemoEditor(AuDemoProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&la2aLookAndFeel);

    // Title - TELETRONIX branding
    titleLabel.setText("TELETRONIX", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::italic | juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, LA2ALookAndFeel::TELETRONIX_RED);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("LEVELING AMPLIFIER", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(8.0f));
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, LA2ALookAndFeel::TEXT_DARK);
    addAndMakeVisible(subtitleLabel);

    // VU Meter
    addAndMakeVisible(vuMeter);

    // Meter mode button - small, top right area
    meterModeButton.setColour(juce::TextButton::buttonColourId, LA2ALookAndFeel::TEXT_DARK);
    meterModeButton.setColour(juce::TextButton::textColourOffId, LA2ALookAndFeel::FACEPLATE);
    meterModeButton.onClick = [this]() {
        if (vuMeter.getMode() == VUMeter::Mode::GainReduction)
        {
            vuMeter.setMode(VUMeter::Mode::Output);
            meterModeButton.setButtonText("OUT");
        }
        else
        {
            vuMeter.setMode(VUMeter::Mode::GainReduction);
            meterModeButton.setButtonText("GR");
        }
    };
    addAndMakeVisible(meterModeButton);

    // Gain knob - large, left side
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                                   juce::MathConstants<float>::pi * 2.25f, true);
    addAndMakeVisible(gainSlider);

    gainLabel.setText("GAIN", juce::dontSendNotification);
    gainLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);

    // Peak Reduction knob - large, right of center
    peakReductionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    peakReductionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    peakReductionSlider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                                            juce::MathConstants<float>::pi * 2.25f, true);
    addAndMakeVisible(peakReductionSlider);

    peakReductionLabel.setText("PEAK REDUCTION", juce::dontSendNotification);
    peakReductionLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    peakReductionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(peakReductionLabel);

    // Mix knob - smaller, far right
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixSlider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                                  juce::MathConstants<float>::pi * 2.25f, true);
    addAndMakeVisible(mixSlider);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setFont(juce::Font(8.0f, juce::Font::bold));
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    // Limit/Compress switch - far left
    limitModeSwitch.setButtonText("");
    addAndMakeVisible(limitModeSwitch);

    // Parameter attachments
    peakReductionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "peakReduction", peakReductionSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "gain", gainSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "mix", mixSlider);
    limitModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getApvts(), "limitMode", limitModeSwitch);

    startTimerHz(30);

    // Rack-mount proportions
    setSize(800, 250);
}

AuDemoEditor::~AuDemoEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void AuDemoEditor::timerCallback()
{
    if (vuMeter.getMode() == VUMeter::Mode::GainReduction)
        vuMeter.setLevel(processorRef.getGainReductionDb());
    else
        vuMeter.setLevel(processorRef.getOutputLevel());
}

void AuDemoEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Black rack ears on sides
    float earWidth = 25.0f;
    g.setColour(LA2ALookAndFeel::RACK_EAR);
    g.fillRect(0.0f, 0.0f, earWidth, bounds.getHeight());
    g.fillRect(bounds.getWidth() - earWidth, 0.0f, earWidth, bounds.getHeight());

    // Rack ear slots
    g.setColour(juce::Colour(0xFF333333));
    for (int i = 0; i < 3; ++i)
    {
        float slotY = 30.0f + i * 80.0f;
        g.fillRoundedRectangle(8.0f, slotY, 10.0f, 25.0f, 3.0f);
        g.fillRoundedRectangle(bounds.getWidth() - 18.0f, slotY, 10.0f, 25.0f, 3.0f);
    }

    // Main faceplate area
    auto faceplate = bounds.reduced(earWidth, 0.0f);

    // Faceplate gradient
    juce::ColourGradient faceplateGradient(
        juce::Colour(0xFFC8C8C8), faceplate.getX(), faceplate.getY(),
        juce::Colour(0xFFAAAAAA), faceplate.getX(), faceplate.getBottom(), false);
    g.setGradientFill(faceplateGradient);
    g.fillRect(faceplate);

    // Top edge highlight
    g.setColour(juce::Colour(0xFFDDDDDD));
    g.fillRect(faceplate.getX(), faceplate.getY(), faceplate.getWidth(), 3.0f);

    // Bottom shadow
    g.setColour(juce::Colour(0xFF888888));
    g.fillRect(faceplate.getX(), faceplate.getBottom() - 3.0f, faceplate.getWidth(), 3.0f);

    // Corner screws
    float screwRadius = 5.0f;
    auto drawScrew = [&](float cx, float cy) {
        // Screw shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillEllipse(cx - screwRadius + 1, cy - screwRadius + 1, screwRadius * 2, screwRadius * 2);
        // Screw body
        juce::ColourGradient screwGrad(
            juce::Colour(0xFFCCCCCC), cx - screwRadius, cy - screwRadius,
            juce::Colour(0xFF888888), cx + screwRadius, cy + screwRadius, true);
        g.setGradientFill(screwGrad);
        g.fillEllipse(cx - screwRadius, cy - screwRadius, screwRadius * 2, screwRadius * 2);
        // Slot
        g.setColour(juce::Colour(0xFF444444));
        g.fillRect(cx - screwRadius * 0.7f, cy - 1.0f, screwRadius * 1.4f, 2.0f);
    };

    float screwInset = 15.0f;
    drawScrew(faceplate.getX() + screwInset, faceplate.getY() + screwInset);
    drawScrew(faceplate.getRight() - screwInset, faceplate.getY() + screwInset);
    drawScrew(faceplate.getX() + screwInset, faceplate.getBottom() - screwInset);
    drawScrew(faceplate.getRight() - screwInset, faceplate.getBottom() - screwInset);

    // Draw knob scale markings on faceplate
    drawKnobScale(g, gainKnobCenter, gainKnobRadius + 15.0f, 0, 100);
    drawKnobScale(g, peakReductionKnobCenter, peakReductionKnobRadius + 15.0f, 0, 100);
}

void AuDemoEditor::drawKnobScale(juce::Graphics& g, juce::Point<float> center,
                                  float radius, int minVal, int maxVal)
{
    g.setColour(LA2ALookAndFeel::TEXT_DARK);
    g.setFont(9.0f);

    float startAngle = juce::MathConstants<float>::pi * 0.75f;
    float endAngle = juce::MathConstants<float>::pi * 2.25f;

    // Draw tick marks and numbers
    for (int i = 0; i <= 10; ++i)
    {
        float normalized = i / 10.0f;
        float angle = startAngle + normalized * (endAngle - startAngle);

        float tickInner = radius - 8.0f;
        float tickOuter = radius;

        float cosA = std::cos(angle - juce::MathConstants<float>::halfPi);
        float sinA = std::sin(angle - juce::MathConstants<float>::halfPi);

        float x1 = center.x + tickInner * cosA;
        float y1 = center.y + tickInner * sinA;
        float x2 = center.x + tickOuter * cosA;
        float y2 = center.y + tickOuter * sinA;

        bool major = (i % 2 == 0);
        g.drawLine(x1, y1, x2, y2, major ? 1.5f : 1.0f);

        // Number labels for major ticks
        if (major)
        {
            int value = minVal + (i * (maxVal - minVal)) / 10;
            float labelRadius = radius + 10.0f;
            float lx = center.x + labelRadius * cosA;
            float ly = center.y + labelRadius * sinA;

            g.drawText(juce::String(value),
                      static_cast<int>(lx - 12), static_cast<int>(ly - 6), 24, 12,
                      juce::Justification::centred);
        }
    }
}

void AuDemoEditor::resized()
{
    auto bounds = getLocalBounds();
    float earWidth = 25.0f;
    auto faceplate = bounds.toFloat().reduced(earWidth, 0.0f);

    // Title area - top left
    titleLabel.setBounds(static_cast<int>(faceplate.getX() + 40), 15, 140, 22);
    subtitleLabel.setBounds(static_cast<int>(faceplate.getX() + 40), 35, 140, 14);

    // Layout: Switch | Gain | VU Meter | Peak Reduction | Mix/Mode
    float contentY = 60.0f;
    float contentHeight = bounds.getHeight() - 80.0f;

    // Limit/Compress switch - far left
    limitModeSwitch.setBounds(static_cast<int>(faceplate.getX() + 35),
                               static_cast<int>(contentY + 20), 50, 100);

    // Gain knob - left of center
    int gainKnobSize = 120;
    int gainX = static_cast<int>(faceplate.getX() + 120);
    int gainY = static_cast<int>(contentY + (contentHeight - gainKnobSize) / 2);
    gainSlider.setBounds(gainX, gainY, gainKnobSize, gainKnobSize);
    gainLabel.setBounds(gainX, gainY + gainKnobSize - 5, gainKnobSize, 20);
    gainKnobCenter = {gainX + gainKnobSize / 2.0f, gainY + gainKnobSize / 2.0f};
    gainKnobRadius = gainKnobSize / 2.0f - 10.0f;

    // VU Meter - center
    int meterWidth = 180;
    int meterHeight = 120;
    int meterX = static_cast<int>(faceplate.getCentreX() - meterWidth / 2);
    int meterY = static_cast<int>(contentY + (contentHeight - meterHeight) / 2 - 10);
    vuMeter.setBounds(meterX, meterY, meterWidth, meterHeight);

    // Meter mode button
    meterModeButton.setBounds(meterX + meterWidth / 2 - 20, meterY + meterHeight + 5, 40, 18);

    // Peak Reduction knob - right of center
    int prKnobSize = 120;
    int prX = static_cast<int>(faceplate.getRight() - 280);
    int prY = static_cast<int>(contentY + (contentHeight - prKnobSize) / 2);
    peakReductionSlider.setBounds(prX, prY, prKnobSize, prKnobSize);
    peakReductionLabel.setBounds(prX - 10, prY + prKnobSize - 5, prKnobSize + 20, 20);
    peakReductionKnobCenter = {prX + prKnobSize / 2.0f, prY + prKnobSize / 2.0f};
    peakReductionKnobRadius = prKnobSize / 2.0f - 10.0f;

    // Mix knob - far right, smaller
    int mixKnobSize = 60;
    int mixX = static_cast<int>(faceplate.getRight() - 100);
    int mixY = static_cast<int>(contentY + 20);
    mixSlider.setBounds(mixX, mixY, mixKnobSize, mixKnobSize);
    mixLabel.setBounds(mixX, mixY + mixKnobSize, mixKnobSize, 16);
}
