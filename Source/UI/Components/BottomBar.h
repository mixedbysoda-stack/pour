#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace pour {

// Bottom toolbar: UNDO | REDO | A | B | [◂ Preset ▸] | Carbonated logo.
class BottomBar : public juce::Component {
public:
    BottomBar();

    void setPresetName(const juce::String& name) { presetName = name; repaint(); }
    void setAB(char s)                             { abSlot = s; repaint(); }
    void setCanUndo(bool v)                        { canUndo = v; repaint(); }
    void setCanRedo(bool v)                        { canRedo = v; repaint(); }

    std::function<void()> onUndo;
    std::function<void()> onRedo;
    std::function<void()> onAClicked;
    std::function<void()> onBClicked;
    std::function<void()> onPrevPreset;
    std::function<void()> onNextPreset;
    std::function<void(juce::Point<int>)> onPresetClicked;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    juce::String presetName = "Default Preset";
    char abSlot = 'A';
    bool canUndo = false, canRedo = false;

    juce::Rectangle<int> rUndo, rRedo, rA, rB, rPrev, rNext, rPreset, rLogo;
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomBar)
};

} // namespace pour
