#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace pour {

// Rotary knob matching the Magic Patterns design: brushed metal bezel,
// tick ring with labels, cyan glow cap variant or neutral pointer variant.
class PourKnob : public juce::Component {
public:
    enum class Variant { Neutral, Cyan };

    struct Tick { float value; juce::String label; };

    PourKnob();

    void setVariant(Variant v) { variant = v; repaint(); }
    void setTicks(std::vector<Tick> t) { ticks = std::move(t); repaint(); }
    void setRange(float min, float max, float defaultValue);
    void setValue(float v, juce::NotificationType nt = juce::sendNotification);
    float getValue() const noexcept { return value; }
    void setValueFromNormalised(float n, juce::NotificationType nt = juce::sendNotification);

    // Attach to APVTS parameter
    void attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);

    std::function<void(float)> onValueChange;

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    Variant variant = Variant::Neutral;
    std::vector<Tick> ticks;

    float rangeMin = -1.f, rangeMax = 1.f, defaultValue = 0.f;
    float value = 0.f;

    // Drag state
    float dragStartValue = 0.f;
    int dragStartY = 0;
    bool dragging = false;

    // APVTS hookup
    juce::AudioProcessorValueTreeState* apvtsPtr = nullptr;
    juce::String paramID;
    std::unique_ptr<juce::ParameterAttachment> attachment;

    float normalised() const noexcept {
        return (rangeMax > rangeMin) ? (value - rangeMin) / (rangeMax - rangeMin) : 0.f;
    }
    float fromNormalised(float n) const noexcept {
        return rangeMin + juce::jlimit(0.f, 1.f, n) * (rangeMax - rangeMin);
    }
    float angleRadians() const;

    void notifyValue();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PourKnob)
};

} // namespace pour
