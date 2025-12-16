#include "VUMeter.h"
#include "LA2ALookAndFeel.h"

VUMeter::VUMeter()
{
    // Calculate smoothing coefficient for VU ballistics
    float framesPerIntegration = (INTEGRATION_TIME_MS / 1000.0f) * FRAME_RATE;
    smoothingCoeff = std::exp(-1.0f / framesPerIntegration);

    startTimerHz(static_cast<int>(FRAME_RATE));
}

VUMeter::~VUMeter()
{
    stopTimer();
}

void VUMeter::setLevel(float dB)
{
    targetLevel = dB;
}

void VUMeter::setMode(Mode newMode)
{
    mode = newMode;
    repaint();
}

void VUMeter::timerCallback()
{
    currentLevel = smoothingCoeff * currentLevel + (1.0f - smoothingCoeff) * targetLevel;
    needleAngle = levelToAngle(currentLevel);
    repaint();
}

float VUMeter::levelToAngle(float dB)
{
    float displayDb = dB;
    if (mode == Mode::GainReduction)
    {
        displayDb = -dB;
    }

    displayDb = juce::jlimit(MIN_DB, MAX_DB, displayDb);
    float normalized = (displayDb - MIN_DB) / (MAX_DB - MIN_DB);
    return -45.0f + normalized * 90.0f;
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    drawMeterFace(g, bounds);
    drawScale(g, bounds);
    drawNeedle(g, bounds, needleAngle);
}

void VUMeter::drawMeterFace(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Outer bezel - dark blue/gray like the reference
    g.setColour(LA2ALookAndFeel::METER_BEZEL);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Inner bezel shadow
    auto innerBezel = bounds.reduced(4.0f);
    g.setColour(juce::Colour(0xFF3A4555));
    g.fillRoundedRectangle(innerBezel, 3.0f);

    // Meter face - cream/yellow gradient like the reference
    auto faceBounds = innerBezel.reduced(4.0f);
    juce::ColourGradient faceGradient(
        juce::Colour(0xFFFAF0D8), faceBounds.getX(), faceBounds.getY(),
        juce::Colour(0xFFE8D8B0), faceBounds.getX(), faceBounds.getBottom(), false);
    g.setGradientFill(faceGradient);
    g.fillRoundedRectangle(faceBounds, 2.0f);

    // "VU LEVEL INDICATOR" text at top
    g.setColour(juce::Colour(0xFF8B0000));
    g.setFont(8.0f);
    g.drawText("VU LEVEL INDICATOR", faceBounds.removeFromTop(14), juce::Justification::centred);

    // "TELETRONIX" branding in meter
    g.setColour(LA2ALookAndFeel::TELETRONIX_RED);
    g.setFont(juce::Font(10.0f, juce::Font::italic | juce::Font::bold));
    auto brandArea = bounds.withHeight(20.0f).withY(bounds.getBottom() - 35.0f);
    g.drawText("TELETRONIX", brandArea, juce::Justification::centred);
}

void VUMeter::drawScale(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto centerX = bounds.getCentreX();
    auto arcRadius = bounds.getWidth() * 0.38f;
    auto arcCenterY = bounds.getBottom() - bounds.getHeight() * 0.25f;

    // VU scale markings
    struct ScaleMark { float dB; const char* label; bool major; bool red; };
    std::vector<ScaleMark> marks = {
        {-20.0f, "-20", true, false},
        {-10.0f, "-10", true, false},
        {-7.0f, "-7", false, false},
        {-5.0f, "-5", false, false},
        {-3.0f, "-3", false, false},
        {-2.0f, "-2", false, false},
        {-1.0f, "-1", false, false},
        {0.0f, "0", true, false},
        {1.0f, "+1", false, true},
        {2.0f, "+2", false, true},
        {3.0f, "+3", true, true}
    };

    g.setFont(9.0f);

    for (const auto& mark : marks)
    {
        float angle = levelToAngle(mark.dB);
        float radians = juce::degreesToRadians(angle - 90.0f);

        float tickInner = arcRadius - (mark.major ? 12.0f : 8.0f);
        float tickOuter = arcRadius - 2.0f;

        float x1 = centerX + tickInner * std::cos(radians);
        float y1 = arcCenterY + tickInner * std::sin(radians);
        float x2 = centerX + tickOuter * std::cos(radians);
        float y2 = arcCenterY + tickOuter * std::sin(radians);

        g.setColour(mark.red ? juce::Colour(0xFFCC0000) : juce::Colour(0xFF333333));
        g.drawLine(x1, y1, x2, y2, mark.major ? 1.5f : 1.0f);

        // Labels for major marks
        if (mark.major)
        {
            float labelRadius = arcRadius - 20.0f;
            float lx = centerX + labelRadius * std::cos(radians);
            float ly = arcCenterY + labelRadius * std::sin(radians);

            g.drawText(mark.label,
                      static_cast<int>(lx - 12), static_cast<int>(ly - 6), 24, 12,
                      juce::Justification::centred);
        }
    }

    // "VU" labels on sides
    g.setColour(juce::Colour(0xFF333333));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("VU", static_cast<int>(centerX - arcRadius - 5), static_cast<int>(arcCenterY - 8), 20, 16,
              juce::Justification::centred);
    g.drawText("VU", static_cast<int>(centerX + arcRadius - 15), static_cast<int>(arcCenterY - 8), 20, 16,
              juce::Justification::centred);
}

void VUMeter::drawNeedle(juce::Graphics& g, juce::Rectangle<float> bounds, float angle)
{
    auto centerX = bounds.getCentreX();
    auto arcCenterY = bounds.getBottom() - bounds.getHeight() * 0.25f;
    auto needleLength = bounds.getWidth() * 0.35f;

    float radians = juce::degreesToRadians(angle - 90.0f);

    // Needle shadow
    g.setColour(juce::Colour(0x30000000));
    float shadowX = centerX + 2.0f + needleLength * std::cos(radians);
    float shadowY = arcCenterY + 2.0f + needleLength * std::sin(radians);
    g.drawLine(centerX + 2.0f, arcCenterY + 2.0f, shadowX, shadowY, 1.5f);

    // Needle - red/dark color
    g.setColour(juce::Colour(0xFF8B0000));
    float tipX = centerX + needleLength * std::cos(radians);
    float tipY = arcCenterY + needleLength * std::sin(radians);
    g.drawLine(centerX, arcCenterY, tipX, tipY, 1.5f);

    // Needle pivot
    g.setColour(juce::Colour(0xFF333333));
    g.fillEllipse(centerX - 4.0f, arcCenterY - 4.0f, 8.0f, 8.0f);
}

void VUMeter::resized()
{
}
