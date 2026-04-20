#include "PourLookAndFeel.h"

namespace pour {

const juce::Colour Colors::bg             = juce::Colour::fromRGB( 20,  22,  25);
const juce::Colour Colors::panelTop       = juce::Colour::fromRGB( 30,  32,  36);
const juce::Colour Colors::panelBottom    = juce::Colour::fromRGB( 26,  28,  31);
const juce::Colour Colors::divider        = juce::Colour::fromRGBA(255,255,255,  12);

const juce::Colour Colors::knobFaceTop    = juce::Colour::fromRGB( 74,  77,  82);
const juce::Colour Colors::knobFaceBottom = juce::Colour::fromRGB( 21,  22,  26);
const juce::Colour Colors::knobRing       = juce::Colour::fromRGB( 46,  49,  53);
const juce::Colour Colors::knobTick       = juce::Colour::fromRGBA(255,255,255, 140);
const juce::Colour Colors::knobLabel      = juce::Colour::fromRGBA(220,225,230, 190);

const juce::Colour Colors::cyan           = juce::Colour::fromRGB( 79, 184, 212);
const juce::Colour Colors::cyanBright     = juce::Colour::fromRGB(184, 236, 255);
const juce::Colour Colors::textPrimary    = juce::Colour::fromRGB(250, 250, 252);
const juce::Colour Colors::textSecondary  = juce::Colour::fromRGB(156, 163, 170);

const juce::Colour Colors::orange         = juce::Colour::fromRGB(232, 160,  48);
const juce::Colour Colors::green          = juce::Colour::fromRGB(111, 220,  92);
const juce::Colour Colors::amber          = juce::Colour::fromRGB(216, 138,  26);
const juce::Colour Colors::red            = juce::Colour::fromRGB(199,  47,  47);

const juce::Colour Colors::meterOffGreen  = juce::Colour::fromRGB( 18,  37,  26);
const juce::Colour Colors::meterOffAmber  = juce::Colour::fromRGB( 42,  31,  13);
const juce::Colour Colors::meterOffRed    = juce::Colour::fromRGB( 42,  16,  16);

const juce::Colour Colors::scopeBg1       = juce::Colour::fromRGB( 21,  52,  62);
const juce::Colour Colors::scopeBg2       = juce::Colour::fromRGB(  8,  20,  25);

PourLookAndFeel::PourLookAndFeel() {
    setColour(juce::PopupMenu::backgroundColourId, Colors::bg);
    setColour(juce::PopupMenu::textColourId, Colors::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::cyan.withAlpha(0.18f));
    setColour(juce::PopupMenu::highlightedTextColourId, Colors::cyanBright);
    setColour(juce::PopupMenu::headerTextColourId, Colors::textSecondary);
}

juce::Font PourLookAndFeel::getLabelFont(juce::Label&) {
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 11.0f, juce::Font::plain);
}
juce::Font PourLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 11.0f, juce::Font::bold);
}
juce::Font PourLookAndFeel::getPopupMenuFont() {
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 13.0f, juce::Font::plain);
}
void PourLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h) {
    g.fillAll(Colors::bg);
    g.setColour(Colors::divider);
    g.drawRect(0, 0, w, h);
}

} // namespace pour
