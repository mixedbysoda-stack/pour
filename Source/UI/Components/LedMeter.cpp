#include "LedMeter.h"
#include "../LookAndFeel/PourLookAndFeel.h"

namespace pour {

LedMeter::LedMeter() {
    setOpaque(false);
    startTimerHz(30);
}

LedMeter::~LedMeter() { stopTimer(); }

void LedMeter::setLevels(float l, float r) {
    // Instant attack, gradual fall
    peakL = std::max(peakL * 0.88f, l);
    peakR = std::max(peakR * 0.88f, r);

    const auto now = juce::Time::getMillisecondCounter();
    if (l > holdL) { holdL = l; holdLTime = now; }
    else if (now - holdLTime > holdMs) holdL = std::max(l, holdL - 0.01f);

    if (r > holdR) { holdR = r; holdRTime = now; }
    else if (now - holdRTime > holdMs) holdR = std::max(r, holdR - 0.01f);
}

void LedMeter::timerCallback() { repaint(); }

static float levelToFrac(float linear) {
    if (linear <= 0.0f) return 0.0f;
    // Map linear 0..1 onto a vaguely-dB curve so quiet signals show mid-bar activity
    const float db = 20.0f * std::log10(std::max(linear, 1.0e-5f));
    return juce::jlimit(0.0f, 1.0f, (db + 48.0f) / 48.0f); // -48..0 dB onto 0..1
}

static std::tuple<juce::Colour, juce::Colour> segColors(int fromTop) {
    if (fromTop < 2) return { juce::Colour::fromRGB(255,106,90), juce::Colour::fromRGB(199,47,47) };
    if (fromTop < 6) return { juce::Colour::fromRGB(255,195,77), juce::Colour::fromRGB(216,138,26) };
    return                  { juce::Colour::fromRGB(122,230,104), juce::Colour::fromRGB(76,175,61) };
}

void LedMeter::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds();
    const int w = bounds.getWidth();
    const int h = bounds.getHeight();

    // Scale labels column ~ 18px, bar area the rest
    constexpr int scaleW = 18;
    const int barAreaW = w - scaleW - 4;
    auto scaleArea = juce::Rectangle<int>(0, 0, scaleW, h);
    auto barArea   = juce::Rectangle<int>(scaleW + 4, 0, barAreaW, h).reduced(1, 1);

    // Scale labels
    g.setColour(Colors::textSecondary);
    g.setFont(8.5f);
    const char* scale[] = { "0", "3", "6", "9", "12", "15", "18", "24", "36", juce::String::charToString((juce::juce_wchar) 0x221E).toRawUTF8() };
    const int nScale = (int) (sizeof(scale) / sizeof(scale[0]));
    for (int i = 0; i < nScale; ++i) {
        const float y = juce::jmap((float) i / (float)(nScale - 1), 0.0f, (float) h - 10.0f);
        g.drawText(scale[i], juce::Rectangle<int>(0, (int) y, scaleW, 10), juce::Justification::centredRight, false);
    }

    // Bar well
    g.setColour(juce::Colour::fromRGB(10, 11, 13));
    g.fillRoundedRectangle(barArea.toFloat(), 3.0f);

    // Two bars
    const int gap = 2;
    const int barW = (barArea.getWidth() - gap) / 2;
    auto drawBar = [&](int xOffset, float level, float hold) {
        const float lFrac = levelToFrac(level);
        const float hFrac = levelToFrac(hold);
        const int lit = juce::roundToInt(lFrac * segments);
        const int peakIdx = std::max(0, juce::roundToInt(hFrac * segments) - 1);

        const int segGap = 1;
        const float segH = (float) (barArea.getHeight() - segGap * (segments - 1)) / (float) segments;

        for (int i = 0; i < segments; ++i) {
            const int fromTop = segments - 1 - i;
            const float y = (float) barArea.getY() + (segments - 1 - i) * (segH + segGap);
            auto [onC, onC2] = segColors(fromTop);
            const bool on = i < lit;
            const bool peak = (i == peakIdx) && hFrac > 0.02f && !on;

            juce::Colour c;
            if (on) c = onC;
            else if (peak) c = onC2;
            else {
                if (fromTop < 2) c = Colors::meterOffRed;
                else if (fromTop < 6) c = Colors::meterOffAmber;
                else c = Colors::meterOffGreen;
            }
            g.setColour(c);
            g.fillRect(juce::Rectangle<float>(
                (float) (barArea.getX() + xOffset), y, (float) barW, segH));
        }
    };
    drawBar(0, peakL, holdL);
    drawBar(barW + gap, peakR, holdR);
}

} // namespace pour
