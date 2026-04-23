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

void PourEditor::parentHierarchyChanged() {
    juce::AudioProcessorEditor::parentHierarchyChanged();

   #if JUCE_WINDOWS
    // Force JUCE 8's GDI software renderer instead of the new Direct2D backend.
    // Direct2D context init crashes on Intel Iris Xe integrated graphics inside
    // sandboxed plug-in host processes (Bitwig PluginHost, Reaper plug-in
    // scanner, MuLab) — confirmed via repeated 0xC0000005 access violations
    // at scan time, despite earlier hotfixes that addressed unrelated raw-pixel
    // and licensing-init paths. Pour exercises the paint path harder than the
    // other Carbonated plug-ins (45 Hz Goniometer + two LedMeters + ARGB images
    // recreated on every resize), which is why only Pour hits this. Software
    // renderer adds negligible CPU cost and is rock-solid across GPU configs.
    if (auto* peer = getPeer()) {
        const auto engines = peer->getAvailableRenderingEngines();
        const int softIdx = engines.indexOf("Software Renderer");
        if (softIdx >= 0 && peer->getCurrentRenderingEngine() != softIdx)
            peer->setCurrentRenderingEngine(softIdx);
    }
   #endif
}

} // namespace pour
