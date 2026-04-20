#include "PourKnob.h"
#include "../LookAndFeel/PourLookAndFeel.h"

namespace pour {

static constexpr float kSweepDeg = 270.0f;
static constexpr float kHalfSweepRad = juce::MathConstants<float>::pi * (kSweepDeg / 2.0f) / 180.0f;

PourKnob::PourKnob() {
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    setWantsKeyboardFocus(false);
}

void PourKnob::setRange(float min, float max, float def) {
    rangeMin = min; rangeMax = max; defaultValue = def;
    value = juce::jlimit(min, max, value);
    repaint();
}

void PourKnob::setValue(float v, juce::NotificationType nt) {
    const float clamped = juce::jlimit(rangeMin, rangeMax, v);
    if (std::abs(clamped - value) < 1.0e-6f) return;
    value = clamped;
    repaint();
    if (nt != juce::dontSendNotification) notifyValue();
}

void PourKnob::setValueFromNormalised(float n, juce::NotificationType nt) {
    setValue(fromNormalised(n), nt);
}

void PourKnob::attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& id) {
    apvtsPtr = &apvts;
    paramID = id;
    attachment = std::make_unique<juce::ParameterAttachment>(
        *apvts.getParameter(id),
        [this](float v) {
            if (std::abs(v - value) > 1.0e-6f) {
                value = juce::jlimit(rangeMin, rangeMax, v);
                repaint();
            }
        },
        nullptr);
    attachment->sendInitialUpdate();
}

void PourKnob::notifyValue() {
    if (attachment) attachment->setValueAsCompleteGesture(value);
    if (onValueChange) onValueChange(value);
}

float PourKnob::angleRadians() const {
    const float n = normalised(); // 0..1
    const float frac = n * 2.0f - 1.0f; // -1..+1
    return frac * kHalfSweepRad;
}

void PourKnob::mouseDown(const juce::MouseEvent& e) {
    if (attachment) attachment->beginGesture();
    dragStartValue = value;
    dragStartY = e.y;
    dragging = true;
}

void PourKnob::mouseDrag(const juce::MouseEvent& e) {
    if (!dragging) return;
    const float sensitivity = e.mods.isShiftDown() ? 400.0f : 150.0f;
    const float dy = (float)(dragStartY - e.y);
    const float deltaNorm = dy / sensitivity;
    const float range = rangeMax - rangeMin;
    float next = dragStartValue + deltaNorm * range;
    next = juce::jlimit(rangeMin, rangeMax, next);
    if (std::abs(next - value) > 1.0e-6f) {
        value = next;
        repaint();
        if (attachment) attachment->setValueAsPartOfGesture(value);
        if (onValueChange) onValueChange(value);
    }
}

void PourKnob::mouseDoubleClick(const juce::MouseEvent&) {
    setValue(defaultValue, juce::sendNotification);
}

void PourKnob::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& w) {
    const float step = (rangeMax - rangeMin) * 0.02f;
    const float next = juce::jlimit(rangeMin, rangeMax,
        value + (w.deltaY > 0.0f ? step : -step));
    setValue(next, juce::sendNotification);
}

void PourKnob::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();

    // Reserve outer 22px for labels
    const float labelMargin = 22.0f;
    const float diameter = std::min(w, h) - labelMargin * 2.0f;
    const float cx = w * 0.5f, cy = h * 0.5f;
    const float knobR = diameter * 0.5f;

    // Tick ring + labels
    if (!ticks.empty()) {
        g.setFont(9.0f);
        const int count = (int) ticks.size();
        for (int i = 0; i < count; ++i) {
            const float frac = (count == 1) ? 0.5f : ((float) i / (float)(count - 1));
            const float a = (frac * 2.f - 1.f) * kHalfSweepRad;
            const float tickInner = knobR + 4.0f;
            const float tickOuter = knobR + 10.0f;
            const float lblR = knobR + 18.0f;

            const float sx = std::sin(a), sy = -std::cos(a);
            const float x1 = cx + sx * tickInner, y1 = cy + sy * tickInner;
            const float x2 = cx + sx * tickOuter, y2 = cy + sy * tickOuter;
            const float lx = cx + sx * lblR,       ly = cy + sy * lblR;

            g.setColour(Colors::knobTick);
            g.drawLine(x1, y1, x2, y2, 1.2f);

            g.setColour(Colors::knobLabel);
            g.drawText(ticks[i].label,
                       juce::Rectangle<float>(lx - 18.0f, ly - 7.0f, 36.0f, 14.0f),
                       juce::Justification::centred, false);
        }
    }

    // Drop shadow
    juce::Path knobPath;
    knobPath.addEllipse(cx - knobR, cy - knobR, knobR * 2, knobR * 2);
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillEllipse(cx - knobR, cy - knobR + 3.0f, knobR * 2, knobR * 2);

    // Outer bezel gradient
    juce::ColourGradient bezel(
        juce::Colour::fromRGB(74, 77, 82), cx, cy - knobR * 0.6f,
        juce::Colour::fromRGB(21, 22, 26), cx, cy + knobR, false);
    g.setGradientFill(bezel);
    g.fillEllipse(cx - knobR, cy - knobR, knobR * 2, knobR * 2);

    // Brushed ring (faux via concentric rings)
    g.setColour(juce::Colour::fromRGB(46, 49, 53));
    g.drawEllipse(cx - knobR, cy - knobR, knobR * 2, knobR * 2, 1.2f);

    // Inner face
    const float innerR = knobR * 0.78f;
    juce::ColourGradient inner(
        juce::Colour::fromRGB(61, 64, 69), cx, cy - innerR * 0.6f,
        juce::Colour::fromRGB(22, 24, 27), cx, cy + innerR, false);
    g.setGradientFill(inner);
    g.fillEllipse(cx - innerR, cy - innerR, innerR * 2, innerR * 2);

    // Subtle concentric rings inside
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    for (float r = innerR * 0.35f; r < innerR * 0.95f; r += 2.5f) {
        g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 0.8f);
    }

    // Top highlight
    juce::ColourGradient hi(
        juce::Colour::fromRGBA(255, 255, 255, 40), cx, cy - innerR * 0.5f,
        juce::Colour::fromRGBA(255, 255, 255, 0),  cx, cy, true);
    g.setGradientFill(hi);
    g.fillEllipse(cx - innerR, cy - innerR, innerR * 2, innerR * 2);

    // Indicator
    const float angle = angleRadians();
    const float sx = std::sin(angle), sy = -std::cos(angle);

    if (variant == Variant::Cyan) {
        const float capR = knobR * 0.22f;
        const float dx = cx + sx * (knobR * 0.55f);
        const float dy = cy + sy * (knobR * 0.55f);
        // Glow halo
        for (int i = 0; i < 3; ++i) {
            const float rr = capR + (float)(i + 1) * 3.0f;
            g.setColour(Colors::cyan.withAlpha(0.18f - i * 0.05f));
            g.fillEllipse(dx - rr, dy - rr, rr * 2, rr * 2);
        }
        juce::ColourGradient cap(
            Colors::cyanBright, dx, dy - capR * 0.4f,
            juce::Colour::fromRGB(27, 109, 133), dx, dy + capR, true);
        g.setGradientFill(cap);
        g.fillEllipse(dx - capR, dy - capR, capR * 2, capR * 2);
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.fillEllipse(dx - capR * 0.35f, dy - capR * 0.5f, capR * 0.6f, capR * 0.4f);
    } else {
        // White pointer line from center outward
        const float lineLen = knobR * 0.70f;
        const float lineStart = knobR * 0.18f;
        const float x1 = cx + sx * lineStart;
        const float y1 = cy + sy * lineStart;
        const float x2 = cx + sx * lineLen;
        const float y2 = cy + sy * lineLen;
        g.setColour(juce::Colours::white);
        g.drawLine(x1, y1, x2, y2, 3.0f);
    }
}

} // namespace pour
