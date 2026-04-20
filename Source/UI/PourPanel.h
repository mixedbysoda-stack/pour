#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Components/PourKnob.h"
#include "Components/LedMeter.h"
#include "Components/Goniometer.h"
#include "Components/PourButton.h"
#include "Components/BottomBar.h"
#include "Components/PresetBrowser.h"

namespace pour {

class PourProcessor;

class PourPanel : public juce::Component, private juce::Timer {
public:
    explicit PourPanel(PourProcessor&);
    ~PourPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void openPresetBrowser(juce::Point<int> screenPt);
    void applyPreset(int idx);
    void syncAB(char to);
    void swapAB(char newSlot);
    void pushUndo();

    PourProcessor& proc;

    // Labels
    juce::Label sectionInput, sectionInputTrim, sectionStereoImage;
    juce::Label sectionShuffle, sectionSpace, sectionWidth;
    juce::Label sectionOutputTrim, sectionOutput;
    juce::Label unitDbIn, unitDbOut, unitShuffleHz, unitSpaceDb, unitWidthDb;
    juce::Label unitInputMS, unitOutputMS;

    // Controls
    PourKnob knobInputTrim, knobOutputTrim, knobShuffle, knobSpace, knobWidth;
    PourButton btnIn, btnSoloSide;
    LedMeter meterIn, meterOut;
    std::unique_ptr<Goniometer> scope;

    // S1-style sliders + readouts
    juce::Slider sliderAsymmetry, sliderRotation;
    juce::Label labelAsymmetry, labelRotation, readoutAsymmetry, readoutRotation;
    std::unique_ptr<juce::SliderParameterAttachment> attAsymmetry, attRotation;

    BottomBar bottomBar;

    // Presets + A/B + Undo
    PresetStore presetStore;
    int currentPresetIdx = 0;
    std::deque<juce::ValueTree> undoStack, redoStack;
    char abSlot = 'A';
    juce::ValueTree otherSlotState;

    std::unique_ptr<juce::CallOutBox> presetCallout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PourPanel)
};

} // namespace pour
