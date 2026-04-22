#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../DSP/StereoImageEngine.h"

namespace pour {

// Half-polar vectorscope. Reads recent L/R samples from the engine ring buffer,
// plots them with a persistence-of-vision fade.
class Goniometer : public juce::Component, private juce::Timer {
public:
    explicit Goniometer(StereoImageEngine& eng);
    ~Goniometer() override;

    // GUI hints to draw the S1-style stereo-field overlay.
    void setOverlay(float widthDb, float rotationDeg, float asymmetryDeg) {
        ovWidthDb = widthDb; ovRotationDeg = rotationDeg; ovAsymmetryDeg = asymmetryDeg;
    }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    StereoImageEngine& engine;
    juce::Image trailLayer;        // persistence layer, faded each tick
    juce::Image trailBack;         // back buffer — enables cross-platform fade
                                   //   via drawImageAt + opacity (no raw pixel
                                   //   access). Previous BitmapData loop crashed
                                   //   FL Studio on Win/Intel Iris Xe at scan.
    std::array<StereoImageEngine::ScopeFrame, 1024> scratch {};

    // Overlay values (set from PourPanel)
    float ovWidthDb = 0.f, ovRotationDeg = 0.f, ovAsymmetryDeg = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Goniometer)
};

} // namespace pour
