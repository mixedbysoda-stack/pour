#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace pour {

// Shared color palette matching the Magic Patterns design.
struct Colors {
    static const juce::Colour bg;            // deep charcoal
    static const juce::Colour panelTop;      // panel gradient top
    static const juce::Colour panelBottom;   // panel gradient bottom
    static const juce::Colour divider;       // subtle bevel line

    static const juce::Colour knobFaceTop;
    static const juce::Colour knobFaceBottom;
    static const juce::Colour knobRing;
    static const juce::Colour knobTick;
    static const juce::Colour knobLabel;

    static const juce::Colour cyan;          // #4fb8d4 — scope + cyan knob cap
    static const juce::Colour cyanBright;    // #b8ecff
    static const juce::Colour textPrimary;   // section labels
    static const juce::Colour textSecondary; // unit labels

    static const juce::Colour orange;        // IN + Solo Side glow
    static const juce::Colour green;         // A button / meter
    static const juce::Colour amber;         // meter amber
    static const juce::Colour red;           // meter red

    static const juce::Colour meterOffGreen;
    static const juce::Colour meterOffAmber;
    static const juce::Colour meterOffRed;

    static const juce::Colour scopeBg1;
    static const juce::Colour scopeBg2;
};

class PourLookAndFeel : public juce::LookAndFeel_V4 {
public:
    PourLookAndFeel();

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;
    void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;
};

} // namespace pour
