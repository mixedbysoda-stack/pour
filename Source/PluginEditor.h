#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel/PourLookAndFeel.h"
#include "UI/PourPanel.h"
#include "Licensing/ActivationDialog.h"

namespace pour {

class PourEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit PourEditor(PourProcessor&);
    ~PourEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    PourProcessor& proc;
    PourLookAndFeel lnf;
    PourPanel panel;
#if !POUR_DEMO
    ActivationDialog activationDialog;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PourEditor)
};

} // namespace pour
