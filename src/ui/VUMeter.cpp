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
    repaint();
}

float VUMeter::levelToAngle(float dB)
{
    // Not used in Dorrough style, but keep for compatibility
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

    // Dark background
    g.setColour(juce::Colour(0xFF0A0A0A));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Inner dark area
    auto innerBounds = bounds.reduced(3.0f);
    g.setColour(juce::Colour(0xFF151515));
    g.fillRoundedRectangle(innerBounds, 4.0f);

    drawDorroughMeter(g, innerBounds);
}

void VUMeter::drawMeterFace(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Not used in Dorrough style
    (void)g;
    (void)bounds;
}

void VUMeter::drawScale(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Not used in Dorrough style
    (void)g;
    (void)bounds;
}

void VUMeter::drawNeedle(juce::Graphics& g, juce::Rectangle<float> bounds, float angle)
{
    // Not used in Dorrough style
    (void)g;
    (void)bounds;
    (void)angle;
}

void VUMeter::drawDorroughMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto centerX = bounds.getCentreX();

    // Calculate radius based on width to ensure it fits horizontally
    // The arc spans from startAngle to endAngle, so we need to account for that
    constexpr float startAngle = -55.0f;  // degrees from top
    constexpr float endAngle = 55.0f;

    // Calculate max radius that fits in width (considering the angle spread)
    // Use smaller multiplier to leave more black space on sides
    float maxAngleRad = juce::degreesToRadians(std::max(std::abs(startAngle), std::abs(endAngle)));
    float maxRadiusForWidth = (bounds.getWidth() * 0.38f) / std::sin(maxAngleRad);
    float maxRadiusForHeight = bounds.getHeight() * 1.1f;

    float arcRadius = std::min(maxRadiusForWidth, maxRadiusForHeight);
    float segmentHeight = bounds.getHeight() * 0.16f;

    // Position pivot so arc fits vertically
    auto centerY = bounds.getBottom() + arcRadius - bounds.getHeight() + segmentHeight;

    // Dorrough scale: -25 to +14 dB (40 segments)
    constexpr int numSegments = 41;
    constexpr float minDb = -25.0f;
    constexpr float maxDb = 14.0f;

    // Get current level for display
    float displayLevel = currentLevel;
    if (mode == Mode::GainReduction)
    {
        displayLevel = -currentLevel;  // Invert for GR display
    }

    // dB values for labels
    struct DbLabel { float dB; const char* label; };
    std::vector<DbLabel> labels = {
        {-25.0f, "-25"}, {-22.0f, "-22"}, {-20.0f, "-20"}, {-18.0f, "-18"},
        {-16.0f, "-16"}, {-14.0f, "-14"}, {-12.0f, "-12"}, {-10.0f, "-10"},
        {-8.0f, "-8"}, {-6.0f, "-6"}, {-4.0f, "-4"}, {-2.0f, "-2"},
        {0.0f, "0"}, {2.0f, "+2"}, {4.0f, "+4"}, {6.0f, "+6"},
        {8.0f, "+8"}, {10.0f, "+10"}, {12.0f, "+12"}, {14.0f, "+14"}
    };

    // Draw LED segments
    for (int i = 0; i < numSegments; ++i)
    {
        float segmentDb = minDb + (static_cast<float>(i) / (numSegments - 1)) * (maxDb - minDb);
        float normalizedPos = static_cast<float>(i) / (numSegments - 1);
        float angle = startAngle + normalizedPos * (endAngle - startAngle);
        float radians = juce::degreesToRadians(angle - 90.0f);

        // Segment position on arc
        float segX = centerX + arcRadius * std::cos(radians);
        float segY = centerY + arcRadius * std::sin(radians);

        // Determine segment color based on position
        juce::Colour segmentColor;
        juce::Colour dimColor;

        if (segmentDb < -10.0f)
        {
            // Green zone
            segmentColor = juce::Colour(0xFF00DD00);
            dimColor = juce::Colour(0xFF0A3A0A);
        }
        else if (segmentDb < 0.0f)
        {
            // Yellow/orange transition zone
            float t = (segmentDb + 10.0f) / 10.0f;
            segmentColor = juce::Colour(0xFF00DD00).interpolatedWith(juce::Colour(0xFFFFAA00), t);
            dimColor = juce::Colour(0xFF0A3A0A).interpolatedWith(juce::Colour(0xFF3A2A0A), t);
        }
        else if (segmentDb < 6.0f)
        {
            // Red zone
            segmentColor = juce::Colour(0xFFFF3300);
            dimColor = juce::Colour(0xFF3A0A0A);
        }
        else
        {
            // High red/yellow zone
            float t = (segmentDb - 6.0f) / 8.0f;
            segmentColor = juce::Colour(0xFFFF3300).interpolatedWith(juce::Colour(0xFFFFCC00), t);
            dimColor = juce::Colour(0xFF3A0A0A).interpolatedWith(juce::Colour(0xFF3A3A0A), t);
        }

        // Check if segment should be lit
        bool isLit = displayLevel >= segmentDb;

        // Draw segment (rotated rectangle)
        juce::Path segment;
        float segWidth = (bounds.getWidth() * 0.8f) / numSegments * 0.7f;

        segment.addRoundedRectangle(-segWidth / 2.0f, -segmentHeight / 2.0f,
                                     segWidth, segmentHeight, 1.5f);

        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(radians + juce::MathConstants<float>::halfPi)
                                             .translated(segX, segY));

        if (isLit)
        {
            // Glow effect for lit segments
            g.setColour(segmentColor.withAlpha(0.3f));
            g.fillRoundedRectangle(-segWidth / 2.0f - 2.0f, -segmentHeight / 2.0f - 2.0f,
                                   segWidth + 4.0f, segmentHeight + 4.0f, 2.5f);

            g.setColour(segmentColor);
        }
        else
        {
            g.setColour(dimColor);
        }
        g.fillPath(segment);

        // Segment highlight for lit segments
        if (isLit)
        {
            g.setColour(segmentColor.brighter(0.3f));
            g.fillRoundedRectangle(-segWidth / 2.0f + 1.0f, -segmentHeight / 2.0f + 1.0f,
                                   segWidth - 2.0f, segmentHeight * 0.3f, 1.0f);
        }

        g.restoreState();
    }

    // Draw dB labels (above the segments, inside the visible area)
    g.setFont(7.0f);
    for (const auto& label : labels)
    {
        float normalizedPos = (label.dB - minDb) / (maxDb - minDb);
        float angle = startAngle + normalizedPos * (endAngle - startAngle);
        float radians = juce::degreesToRadians(angle - 90.0f);

        // Labels positioned closer to segments
        float labelRadius = arcRadius - segmentHeight * 0.5f - 10.0f;
        float lx = centerX + labelRadius * std::cos(radians);
        float ly = centerY + labelRadius * std::sin(radians);

        // Color based on zone
        if (label.dB < 0.0f)
            g.setColour(juce::Colour(0xFFAAAA00));  // Yellow for negative dB
        else
            g.setColour(juce::Colour(0xFFFF6600));  // Orange for positive dB

        g.drawText(label.label,
                  static_cast<int>(lx - 14), static_cast<int>(ly - 5), 28, 10,
                  juce::Justification::centred);
    }

    // "dB" labels on sides (at the arc level)
    float dbLabelRadius = arcRadius - segmentHeight * 0.5f;
    float leftAngle = juce::degreesToRadians(startAngle - 90.0f);
    float rightAngle = juce::degreesToRadians(endAngle - 90.0f);

    g.setColour(juce::Colour(0xFFAAAA00));
    g.setFont(9.0f);
    g.drawText("dB", static_cast<int>(centerX + dbLabelRadius * std::cos(leftAngle) - 18),
               static_cast<int>(centerY + dbLabelRadius * std::sin(leftAngle) - 5), 16, 10,
               juce::Justification::centred);
    g.drawText("dB", static_cast<int>(centerX + dbLabelRadius * std::cos(rightAngle) + 2),
               static_cast<int>(centerY + dbLabelRadius * std::sin(rightAngle) - 5), 16, 10,
               juce::Justification::centred);

    // Mode indicator
    g.setFont(9.0f);
    g.setColour(juce::Colour(0xFF888888));
    const char* modeText = (mode == Mode::GainReduction) ? "GR" : "OUT";
    g.drawText(modeText, bounds.toNearestInt(), juce::Justification::centredBottom);
}

void VUMeter::resized()
{
}
