#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace pour {

class LedMeter : public juce::Component, private juce::Timer {
public:
    LedMeter();
    ~LedMeter() override;

    // Feed new peaks (linear 0..1). Called from timer polling the engine.
    void setLevels(float left, float right);

    void paint(juce::Graphics&) override;
    void resized() override {}

private:
    void timerCallback() override;

    static constexpr int segments = 22;

    float peakL = 0.f, peakR = 0.f;        // decayed peaks for bars
    float holdL = 0.f, holdR = 0.f;        // 3-sec peak hold
    juce::uint32 holdLTime = 0, holdRTime = 0;
    static constexpr juce::uint32 holdMs = 3000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedMeter)
};

} // namespace pour
