#include "PluginEditor.h"

namespace pour {

PourEditor::PourEditor(PourProcessor& p)
    : juce::AudioProcessorEditor(&p), proc(p), panel(p)
#if !POUR_DEMO
      , activationDialog(p.getLicenseManager())
#endif
{
    setLookAndFeel(&lnf);
    addAndMakeVisible(panel);

#if !POUR_DEMO
    addChildComponent(activationDialog);
    if (!proc.getLicenseManager().isActivated())
        activationDialog.setVisible(true);
    startTimerHz(4);
#endif

    setResizable(true, true);
    setResizeLimits(900, 360, 1600, 640);
    setSize(1146, 500);
}

PourEditor::~PourEditor() {
#if !POUR_DEMO
    stopTimer();
#endif
    setLookAndFeel(nullptr);
}

void PourEditor::timerCallback() {
#if !POUR_DEMO
    if (proc.getLicenseManager().isActivated() && activationDialog.isVisible()) {
        activationDialog.setVisible(false);
        repaint();
    }
#endif
}

void PourEditor::paint(juce::Graphics& g) {
    // Deep charcoal vignette, like the Magic Patterns design
    auto r = getLocalBounds().toFloat();
    juce::ColourGradient grad(juce::Colour::fromRGB(28, 30, 34), r.getCentreX(), r.getCentreY(),
                              juce::Colour::fromRGB(10, 11, 13), r.getX(), r.getBottom(), true);
    g.setGradientFill(grad);
    g.fillAll();
}

void PourEditor::resized() {
    panel.setBounds(getLocalBounds().reduced(14));
#if !POUR_DEMO
    activationDialog.setBounds(getLocalBounds());
#endif
}

} // namespace pour
