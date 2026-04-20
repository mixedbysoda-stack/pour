#include "PourButton.h"
#include "../LookAndFeel/PourLookAndFeel.h"

namespace pour {

PourButton::PourButton(const juce::String& label, const juce::String& sub)
    : juce::Button(label), topLine(label), bottomLine(sub)
{
    setClickingTogglesState(true);
}

void PourButton::setLines(const juce::String& top, const juce::String& bottom) {
    topLine = top;
    bottomLine = bottom;
    repaint();
}

void PourButton::attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& id) {
    attachment = std::make_unique<juce::ParameterAttachment>(
        *apvts.getParameter(id),
        [this](float v) { setToggleState(v > 0.5f, juce::dontSendNotification); repaint(); },
        nullptr);
    attachment->sendInitialUpdate();

    onClick = [this]() {
        if (attachment) attachment->setValueAsCompleteGesture(getToggleState() ? 1.0f : 0.0f);
    };
}

void PourButton::paintButton(juce::Graphics& g, bool over, bool down) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float corner = 4.0f;
    const bool on = getToggleState();

    if (on) {
        // Orange glow gradient
        juce::ColourGradient grad(
            juce::Colour::fromRGB(255, 200, 112), bounds.getCentreX(), bounds.getY() + bounds.getHeight() * 0.25f,
            juce::Colour::fromRGB(138,  90,  16), bounds.getCentreX(), bounds.getBottom(), true);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, corner);

        // Outer glow
        for (int i = 0; i < 4; ++i) {
            g.setColour(Colors::orange.withAlpha(0.20f - i * 0.04f));
            g.drawRoundedRectangle(bounds.expanded((float) i + 1.0f), corner + i, 1.0f);
        }

        g.setColour(juce::Colour::fromRGB(40, 22, 0));
    } else {
        juce::ColourGradient grad(
            juce::Colour::fromRGB(58, 61, 66), bounds.getCentreX(), bounds.getY(),
            juce::Colour::fromRGB(36, 38, 42), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, corner);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 0.8f);
        g.setColour(Colors::textSecondary);
    }

    if (over) g.setOpacity(on ? 0.95f : 0.85f);
    if (down) g.setOpacity(0.8f);

    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.5f, juce::Font::bold));
    if (bottomLine.isEmpty()) {
        g.drawText(topLine, bounds, juce::Justification::centred, false);
    } else {
        const auto half = bounds.reduced(2.0f);
        g.drawText(topLine,
                   half.withTop(half.getY()).withHeight(half.getHeight() * 0.5f),
                   juce::Justification::centred, false);
        g.drawText(bottomLine,
                   half.withTop(half.getY() + half.getHeight() * 0.5f).withHeight(half.getHeight() * 0.5f),
                   juce::Justification::centred, false);
    }
}

} // namespace pour
