#include "PluginEditor.h"

AuDemoEditor::AuDemoEditor(AuDemoProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&la2aLookAndFeel);

    // Title - TELETERONIX branding
    titleLabel.setText("TELETERONIX", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f).withStyle("Bold Italic")));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, LA2ALookAndFeel::TELETRONIX_RED);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("JEEJEEING AMPLIFIER", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, LA2ALookAndFeel::TEXT_DARK);
    addAndMakeVisible(subtitleLabel);

    // DEBUG label
    debugLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    debugLabel.setJustificationType(juce::Justification::centredLeft);
    debugLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    addAndMakeVisible(debugLabel);

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

    // Mix fader - horizontal, below meter
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(mixSlider);

    mixLabel.setText("DRY", juce::dontSendNotification);
    mixLabel.setFont(juce::Font(juce::FontOptions(8.0f).withStyle("Bold")));
    mixLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(mixLabel);

    mixLabelWet.setText("WET", juce::dontSendNotification);
    mixLabelWet.setFont(juce::Font(juce::FontOptions(8.0f).withStyle("Bold")));
    mixLabelWet.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(mixLabelWet);

    // LIMIT button - below Peak Reduction
    limitButton.setButtonText("LIMIT");
    addAndMakeVisible(limitButton);

    // COMP button - below Gain
    compButton.setButtonText("COMP");
    addAndMakeVisible(compButton);

    // Parameter attachments
    peakReductionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "peakReduction", peakReductionSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "gain", gainSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getApvts(), "mix", mixSlider);
    limitModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getApvts(), "limitMode", limitButton);
    compModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getApvts(), "compMode", compButton);

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
    float gr = processorRef.getGainReductionDb();
    float out = processorRef.getOutputLevel();

    if (vuMeter.getMode() == VUMeter::Mode::GainReduction)
        vuMeter.setLevel(gr);
    else
        vuMeter.setLevel(out);

    // DEBUG: Show input and metering values
    float inLevel = processorRef.getDebugInputLevel();
    debugLabel.setText("IN: " + juce::String(inLevel, 3) + " | GR: " + juce::String(gr, 1) + " | OUT: " + juce::String(out, 1),
                       juce::dontSendNotification);
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

    // Title area - centered
    titleLabel.setBounds(static_cast<int>(faceplate.getX()), 6, static_cast<int>(faceplate.getWidth()), 28);
    subtitleLabel.setBounds(static_cast<int>(faceplate.getX()), 32, static_cast<int>(faceplate.getWidth()), 16);

    // DEBUG label - bottom left
    debugLabel.setBounds(static_cast<int>(faceplate.getX()) + 10, bounds.getHeight() - 20, 200, 16);

    // VU Meter - center (define first to use for knob positioning)
    int meterWidth = 180;
    int meterHeight = 120;
    int meterX = static_cast<int>(faceplate.getCentreX() - meterWidth / 2);
    int meterY = static_cast<int>((bounds.getHeight() - meterHeight) / 2);
    vuMeter.setBounds(meterX, meterY, meterWidth, meterHeight);

    // Meter mode button
    meterModeButton.setBounds(meterX + meterWidth / 2 - 20, meterY + meterHeight + 5, 40, 18);

    // Gain knob - centered between left faceplate edge and meter left edge
    int gainKnobSize = 100;
    float leftAreaStart = faceplate.getX();
    float leftAreaEnd = static_cast<float>(meterX);
    float gainCenterX = (leftAreaStart + leftAreaEnd) / 2.0f;
    float gainCenterY = bounds.getHeight() / 2.0f;
    int gainX = static_cast<int>(gainCenterX - gainKnobSize / 2.0f);
    int gainY = static_cast<int>(gainCenterY - gainKnobSize / 2.0f);
    gainSlider.setBounds(gainX, gainY, gainKnobSize, gainKnobSize);
    gainLabel.setBounds(gainX, gainY + gainKnobSize - 5, gainKnobSize, 20);
    gainKnobCenter = {gainCenterX, gainCenterY};
    gainKnobRadius = gainKnobSize / 2.0f - 10.0f;

    // Peak Reduction knob - centered between meter right edge and right faceplate edge
    int prKnobSize = 100;
    float rightAreaStart = static_cast<float>(meterX + meterWidth);
    float rightAreaEnd = faceplate.getRight();
    float prCenterX = (rightAreaStart + rightAreaEnd) / 2.0f;
    float prCenterY = bounds.getHeight() / 2.0f;
    int prX = static_cast<int>(prCenterX - prKnobSize / 2.0f);
    int prY = static_cast<int>(prCenterY - prKnobSize / 2.0f);
    peakReductionSlider.setBounds(prX, prY, prKnobSize, prKnobSize);
    peakReductionLabel.setBounds(prX - 10, prY + prKnobSize - 5, prKnobSize + 20, 20);
    peakReductionKnobCenter = {prCenterX, prCenterY};
    peakReductionKnobRadius = prKnobSize / 2.0f - 10.0f;

    // COMP button - below Gain knob (with space for label)
    int buttonWidth = 60;
    int buttonHeight = 24;
    compButton.setBounds(static_cast<int>(gainCenterX - buttonWidth / 2),
                         gainY + gainKnobSize + 20, buttonWidth, buttonHeight);

    // LIMIT button - below Peak Reduction knob (with space for label)
    limitButton.setBounds(static_cast<int>(prCenterX - buttonWidth / 2),
                          prY + prKnobSize + 20, buttonWidth, buttonHeight);

    // Mix fader - horizontal, below the meter
    int mixFaderWidth = 120;
    int mixFaderHeight = 20;
    int mixX = meterX + (meterWidth - mixFaderWidth) / 2;
    int mixY = meterY + meterHeight + 25;
    mixSlider.setBounds(mixX, mixY, mixFaderWidth, mixFaderHeight);
    mixLabel.setBounds(mixX - 32, mixY, 30, mixFaderHeight);
    mixLabelWet.setBounds(mixX + mixFaderWidth + 2, mixY, 30, mixFaderHeight);
}
