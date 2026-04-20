#include "BottomBar.h"
#include "../LookAndFeel/PourLookAndFeel.h"
#include <BinaryData.h>

namespace pour {

BottomBar::BottomBar() {
    setMouseCursor(juce::MouseCursor::NormalCursor);
    logoImage = juce::ImageCache::getFromMemory(
        BinaryData::CarbonatedLogo_png, BinaryData::CarbonatedLogo_pngSize);
}

void BottomBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Bar background
    juce::ColourGradient bg(
        juce::Colour::fromRGB(18, 19, 22), bounds.getCentreX(), bounds.getY(),
        juce::Colour::fromRGB(10, 11, 13), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRect(bounds);

    // Top highlight line
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawHorizontalLine(0, bounds.getX(), bounds.getRight());

    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::bold));

    // UNDO / REDO
    g.setColour(canUndo ? Colors::textPrimary : Colors::textSecondary.withAlpha(0.5f));
    g.drawText("UNDO", rUndo, juce::Justification::centredLeft, false);
    g.setColour(canRedo ? Colors::textPrimary : Colors::textSecondary.withAlpha(0.5f));
    g.drawText("REDO", rRedo, juce::Justification::centredLeft, false);

    // A / B pills
    auto drawPill = [&](juce::Rectangle<int> r, const juce::String& label, bool active) {
        auto rf = r.toFloat();
        if (active) {
            juce::ColourGradient grad(
                juce::Colour::fromRGB(111, 220, 92), rf.getCentreX(), rf.getY(),
                juce::Colour::fromRGB(62, 156, 46),  rf.getCentreX(), rf.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(rf, 3.0f);
            for (int i = 0; i < 3; ++i) {
                g.setColour(Colors::green.withAlpha(0.18f - i * 0.05f));
                g.drawRoundedRectangle(rf.expanded((float)(i + 1)), 3.0f + i, 1.0f);
            }
            g.setColour(juce::Colour::fromRGB(10, 26, 6));
        } else {
            juce::ColourGradient grad(
                juce::Colour::fromRGB(58, 61, 66), rf.getCentreX(), rf.getY(),
                juce::Colour::fromRGB(36, 38, 42), rf.getCentreX(), rf.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(rf, 3.0f);
            g.setColour(Colors::textSecondary);
        }
        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::bold));
        g.drawText(label, r, juce::Justification::centred, false);
    };
    drawPill(rA, "A", abSlot == 'A');
    drawPill(rB, "B", abSlot == 'B');

    // Preset pill
    auto presetF = rPreset.toFloat();
    juce::ColourGradient presetBg(
        juce::Colour::fromRGB(10, 12, 14), presetF.getCentreX(), presetF.getY(),
        juce::Colour::fromRGB(21, 24, 27), presetF.getCentreX(), presetF.getBottom(), false);
    g.setGradientFill(presetBg);
    g.fillRoundedRectangle(presetF, 3.0f);
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawRoundedRectangle(presetF.reduced(0.5f), 3.0f, 1.0f);

    // Chevrons
    auto drawChevron = [&](juce::Rectangle<int> r, bool right) {
        auto rf = r.toFloat().reduced(4.0f);
        g.setColour(Colors::textSecondary);
        juce::Path p;
        if (right) {
            p.startNewSubPath(rf.getX(), rf.getY());
            p.lineTo(rf.getRight(), rf.getCentreY());
            p.lineTo(rf.getX(), rf.getBottom());
        } else {
            p.startNewSubPath(rf.getRight(), rf.getY());
            p.lineTo(rf.getX(), rf.getCentreY());
            p.lineTo(rf.getRight(), rf.getBottom());
        }
        g.strokePath(p, juce::PathStrokeType(1.2f));
    };
    drawChevron(rPrev, false);
    drawChevron(rNext, true);

    // Preset name
    g.setColour(Colors::cyan);
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 11.0f, juce::Font::plain));
    g.drawText(presetName, rPreset, juce::Justification::centred, false);

    // Logo
    if (logoImage.isValid()) {
        const float destH = (float) rLogo.getHeight() - 8.0f;
        const float aspect = (float) logoImage.getWidth() / (float) logoImage.getHeight();
        const float destW = destH * aspect;
        const float destX = (float) rLogo.getRight() - destW;
        const float destY = (float) rLogo.getY() + ((float) rLogo.getHeight() - destH) * 0.5f;
        g.setOpacity(0.92f);
        g.drawImage(logoImage, juce::Rectangle<float>(destX, destY, destW, destH),
                    juce::RectanglePlacement::xRight | juce::RectanglePlacement::yMid);
        g.setOpacity(1.0f);
    }
}

void BottomBar::resized() {
    auto b = getLocalBounds().reduced(10, 0);
    const int h = b.getHeight();

    rUndo = b.removeFromLeft(44).withSizeKeepingCentre(44, h);
    b.removeFromLeft(6);
    rRedo = b.removeFromLeft(44).withSizeKeepingCentre(44, h);
    b.removeFromLeft(10);
    rA = b.removeFromLeft(22).withSizeKeepingCentre(22, 18);
    b.removeFromLeft(2);
    rB = b.removeFromLeft(22).withSizeKeepingCentre(22, 18);

    rLogo = b.removeFromRight(150);

    // Preset pill with chevrons centred in remaining space
    const int presetW = 260;
    auto mid = b;
    int cx = mid.getCentreX();
    rPreset = juce::Rectangle<int>(cx - presetW / 2, mid.getY() + 4, presetW, h - 8);
    rPrev = juce::Rectangle<int>(rPreset.getX() + 6, rPreset.getY(), 18, rPreset.getHeight());
    rNext = juce::Rectangle<int>(rPreset.getRight() - 24, rPreset.getY(), 18, rPreset.getHeight());
}

void BottomBar::mouseUp(const juce::MouseEvent& e) {
    auto p = e.getPosition();
    if (rUndo.contains(p) && canUndo && onUndo) onUndo();
    else if (rRedo.contains(p) && canRedo && onRedo) onRedo();
    else if (rA.contains(p) && onAClicked) onAClicked();
    else if (rB.contains(p) && onBClicked) onBClicked();
    else if (rPrev.contains(p) && onPrevPreset) onPrevPreset();
    else if (rNext.contains(p) && onNextPreset) onNextPreset();
    else if (rPreset.contains(p) && onPresetClicked) onPresetClicked(localPointToGlobal(p));
}

} // namespace pour
