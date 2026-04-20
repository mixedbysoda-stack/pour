#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace pour {

// Bright orange-glow toggle button ("IN", "SOLO SIDE"). Engraved-style when off.
class PourButton : public juce::Button {
public:
    PourButton(const juce::String& label, const juce::String& sub = {});

    void attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);

    void setLines(const juce::String& top, const juce::String& bottom = {});

protected:
    void paintButton(juce::Graphics&, bool over, bool down) override;

private:
    juce::String topLine, bottomLine;
    std::unique_ptr<juce::ParameterAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PourButton)
};

} // namespace pour
