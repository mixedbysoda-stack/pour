#include "PourPanel.h"
#include "../PluginProcessor.h"
#include "LookAndFeel/PourLookAndFeel.h"

namespace pour {

static void styleSectionLabel(juce::Label& l) {
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, Colors::textPrimary);
    l.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.5f, juce::Font::bold));
}
static void styleUnitLabel(juce::Label& l) {
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, Colors::textSecondary);
    l.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 9.5f, juce::Font::plain));
}

PourPanel::PourPanel(PourProcessor& p)
    : proc(p), btnIn("IN"), btnSoloSide("SOLO", "SIDE")
{
    auto& apvts = proc.getState();

    // Section labels
    sectionInput.setText("INPUT", juce::dontSendNotification);
    sectionInputTrim.setText("INPUT TRIM", juce::dontSendNotification);
    sectionStereoImage.setText("STEREO IMAGE", juce::dontSendNotification);
    sectionShuffle.setText("SHUFFLE", juce::dontSendNotification);
    sectionSpace.setText("SPACE", juce::dontSendNotification);
    sectionWidth.setText("WIDTH", juce::dontSendNotification);
    sectionOutputTrim.setText("OUTPUT TRIM", juce::dontSendNotification);
    sectionOutput.setText("OUTPUT", juce::dontSendNotification);
    for (auto* l : { &sectionInput, &sectionInputTrim, &sectionStereoImage,
                     &sectionShuffle, &sectionSpace, &sectionWidth,
                     &sectionOutputTrim, &sectionOutput }) {
        styleSectionLabel(*l);
        addAndMakeVisible(*l);
    }

    unitDbIn.setText("dB", juce::dontSendNotification);
    unitDbOut.setText("dB", juce::dontSendNotification);
    unitShuffleHz.setText("Hz", juce::dontSendNotification);
    unitSpaceDb.setText("dB", juce::dontSendNotification);
    unitWidthDb.setText("dB", juce::dontSendNotification);
    unitInputMS.setText("M / S", juce::dontSendNotification);
    unitOutputMS.setText("M / S", juce::dontSendNotification);
    for (auto* l : { &unitDbIn, &unitDbOut, &unitShuffleHz, &unitSpaceDb, &unitWidthDb,
                     &unitInputMS, &unitOutputMS }) {
        styleUnitLabel(*l);
        addAndMakeVisible(*l);
    }

    // Knobs + ticks
    knobInputTrim.setVariant(PourKnob::Variant::Neutral);
    knobOutputTrim.setVariant(PourKnob::Variant::Neutral);
    knobShuffle.setVariant(PourKnob::Variant::Cyan);
    knobSpace.setVariant(PourKnob::Variant::Cyan);
    knobWidth.setVariant(PourKnob::Variant::Cyan);

    std::vector<PourKnob::Tick> trimTicks = {
        { -12, "-12" }, { -8, "-8" }, { -4, "-4" }, { 0, "0" },
        {   4, "+4" }, {  8, "+8" }, { 12, "+12" }
    };
    std::vector<PourKnob::Tick> widthTicks = {
        { -6, "-6" }, { -4, "-4" }, { -2, "-2" }, { 0, "0" },
        {  2, "+2" }, {  4, "+4" }, {  6, "+6" }
    };
    std::vector<PourKnob::Tick> shuffleTicks = {
        { 40, "40" }, { 100, "100" }, { 160, "160" }, { 220, "220" },
        { 280, "280" }, { 340, "340" }, { 400, "400" }
    };
    knobInputTrim.setRange(-12, 12, 0);
    knobInputTrim.setTicks(trimTicks);
    knobOutputTrim.setRange(-12, 12, 0);
    knobOutputTrim.setTicks(trimTicks);
    knobShuffle.setRange(40, 400, 100);
    knobShuffle.setTicks(shuffleTicks);
    knobSpace.setRange(-12, 12, 0);
    knobSpace.setTicks(trimTicks);
    knobWidth.setRange(-6, 6, 0);
    knobWidth.setTicks(widthTicks);

    knobInputTrim.attach(apvts, params::inputTrim);
    knobOutputTrim.attach(apvts, params::outputTrim);
    knobShuffle.attach(apvts, params::shuffle);
    knobSpace.attach(apvts, params::space);
    knobWidth.attach(apvts, params::width);

    for (auto* k : { &knobInputTrim, &knobOutputTrim, &knobShuffle, &knobSpace, &knobWidth })
        addAndMakeVisible(*k);

    // Buttons
    btnIn.attach(apvts, params::inEnabled);
    btnSoloSide.attach(apvts, params::soloSide);
    addAndMakeVisible(btnIn);
    addAndMakeVisible(btnSoloSide);

    // Meters + scope
    addAndMakeVisible(meterIn);
    addAndMakeVisible(meterOut);
    scope = std::make_unique<Goniometer>(proc.getEngine());
    addAndMakeVisible(*scope);

    // Sliders: Asymmetry vertical (drag up/down), Rotation horizontal.
    auto styleSlider = [](juce::Slider& s, juce::Slider::SliderStyle st) {
        s.setSliderStyle(st);
        s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        s.setColour(juce::Slider::backgroundColourId,    juce::Colour::fromRGB(10, 11, 13));
        s.setColour(juce::Slider::trackColourId,         Colors::cyan.withAlpha(0.55f));
        s.setColour(juce::Slider::thumbColourId,         Colors::cyanBright);
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(10, 11, 13));
        s.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
        s.setColour(juce::Slider::textBoxTextColourId,   Colors::textPrimary);
    };
    styleSlider(sliderAsymmetry, juce::Slider::LinearVertical);
    styleSlider(sliderRotation,  juce::Slider::LinearHorizontal);
    sliderAsymmetry.setDoubleClickReturnValue(true, 0.0);
    sliderRotation.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(sliderAsymmetry);
    addAndMakeVisible(sliderRotation);

    attAsymmetry = std::make_unique<juce::SliderParameterAttachment>(
        *apvts.getParameter(params::asymmetry), sliderAsymmetry);
    attRotation  = std::make_unique<juce::SliderParameterAttachment>(
        *apvts.getParameter(params::rotation),  sliderRotation);

    auto styleText = [](juce::Label& l, juce::Justification j) {
        l.setJustificationType(j);
        l.setColour(juce::Label::textColourId, Colors::textSecondary);
        l.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::bold));
    };
    labelAsymmetry.setText("Asymmetry", juce::dontSendNotification);
    labelRotation.setText("Rotation",  juce::dontSendNotification);
    styleText(labelAsymmetry, juce::Justification::centredLeft);
    styleText(labelRotation,  juce::Justification::centredLeft);
    addAndMakeVisible(labelAsymmetry);
    addAndMakeVisible(labelRotation);

    styleText(readoutAsymmetry, juce::Justification::centredRight);
    styleText(readoutRotation,  juce::Justification::centredRight);
    readoutAsymmetry.setColour(juce::Label::textColourId, Colors::textPrimary);
    readoutRotation.setColour(juce::Label::textColourId, Colors::textPrimary);
    addAndMakeVisible(readoutAsymmetry);
    addAndMakeVisible(readoutRotation);

    auto updateReadouts = [this]() {
        const double a = sliderAsymmetry.getValue();
        if (std::abs(a) < 0.05)
            readoutAsymmetry.setText("Center", juce::dontSendNotification);
        else
            readoutAsymmetry.setText(juce::String(juce::roundToInt(a)), juce::dontSendNotification);
        readoutRotation.setText(juce::String(sliderRotation.getValue(), 1), juce::dontSendNotification);
    };
    sliderAsymmetry.onValueChange = updateReadouts;
    sliderRotation.onValueChange = updateReadouts;
    updateReadouts();

    // Bottom bar hooks
    bottomBar.onUndo        = [this] { if (!undoStack.empty()) {
        redoStack.push_front(proc.getState().copyState());
        auto s = undoStack.back(); undoStack.pop_back();
        proc.getState().replaceState(s);
        bottomBar.setCanUndo(!undoStack.empty());
        bottomBar.setCanRedo(!redoStack.empty());
    } };
    bottomBar.onRedo        = [this] { if (!redoStack.empty()) {
        undoStack.push_back(proc.getState().copyState());
        auto s = redoStack.front(); redoStack.pop_front();
        proc.getState().replaceState(s);
        bottomBar.setCanUndo(!undoStack.empty());
        bottomBar.setCanRedo(!redoStack.empty());
    } };
    bottomBar.onAClicked    = [this] { swapAB('A'); };
    bottomBar.onBClicked    = [this] { swapAB('B'); };
    bottomBar.onPrevPreset  = [this] { int n = (int) presetStore.all().size();
                                        if (n > 0) applyPreset((currentPresetIdx - 1 + n) % n); };
    bottomBar.onNextPreset  = [this] { int n = (int) presetStore.all().size();
                                        if (n > 0) applyPreset((currentPresetIdx + 1) % n); };
    bottomBar.onPresetClicked = [this](juce::Point<int> pt) { openPresetBrowser(pt); };
    addAndMakeVisible(bottomBar);

    abSlot = proc.getABSlot();
    otherSlotState = (abSlot == 'A') ? proc.getABState('B') : proc.getABState('A');
    bottomBar.setAB(abSlot);
    bottomBar.setPresetName(presetStore.all()[currentPresetIdx].name);

    // Parameter-change listener for undo snapshots (coarse)
    apvts.state.addListener(nullptr); // placeholder — undo push happens on gesture end via UI

    startTimerHz(30);
}

PourPanel::~PourPanel() {
    stopTimer();
    proc.setABState(abSlot, proc.getState().copyState());
    proc.setABState(abSlot == 'A' ? 'B' : 'A', otherSlotState);
    proc.setABSlot(abSlot);
}

void PourPanel::paint(juce::Graphics& g) {
    // Panel body
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient body(
        Colors::panelTop, bounds.getCentreX(), bounds.getY(),
        Colors::panelBottom, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(body);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void PourPanel::resized() {
    auto area = getLocalBounds();
    auto bottom = area.removeFromBottom(42);
    bottomBar.setBounds(bottom);

    area.reduce(10, 10);
    area.removeFromTop(4);

    // Column widths
    const int meterW   = 74;
    const int trimKnobW = 132;
    const int sideKnobW = 120;

    auto left = area.removeFromLeft(meterW + trimKnobW + 8);
    auto right = area.removeFromRight(meterW + trimKnobW + 8);
    auto center = area;

    // ---- Left cluster: INPUT meter + INPUT TRIM (+ IN button) ----
    {
        auto l = left;
        // Input meter column
        auto meterCol = l.removeFromLeft(meterW);
        sectionInput.setBounds(meterCol.removeFromTop(18));
        unitInputMS.setBounds(meterCol.removeFromBottom(16));
        meterIn.setBounds(meterCol.reduced(6, 4));

        // Input trim column
        auto trimCol = l;
        sectionInputTrim.setBounds(trimCol.removeFromTop(18));
        auto unitRow = trimCol.removeFromBottom(20);
        unitDbIn.setBounds(unitRow);
        // IN button sits below the knob
        auto btnRow = trimCol.removeFromBottom(46);
        btnIn.setBounds(btnRow.withSizeKeepingCentre(52, 38));
        knobInputTrim.setBounds(trimCol.reduced(6, 2));
    }

    // ---- Right cluster: OUTPUT TRIM (+ SOLO SIDE) + OUTPUT meter ----
    {
        auto r = right;
        auto meterCol = r.removeFromRight(meterW);
        sectionOutput.setBounds(meterCol.removeFromTop(18));
        unitOutputMS.setBounds(meterCol.removeFromBottom(16));
        meterOut.setBounds(meterCol.reduced(6, 4));

        auto trimCol = r;
        sectionOutputTrim.setBounds(trimCol.removeFromTop(18));
        auto unitRow = trimCol.removeFromBottom(20);
        unitDbOut.setBounds(unitRow);
        auto btnRow = trimCol.removeFromBottom(46);
        btnSoloSide.setBounds(btnRow.withSizeKeepingCentre(52, 38));
        knobOutputTrim.setBounds(trimCol.reduced(6, 2));
    }

    // ---- Center cluster: STEREO IMAGE scope + sliders + 3 cyan knobs ----
    {
        // Reserve the knob row first
        auto bottomRow = center.removeFromBottom(130);

        // Carve off the vertical Asymmetry column on the right BEFORE laying out
        // the STEREO IMAGE label and the Rotation slider — so both of those center
        // cleanly over/under the scope area only.
        const int asymColW = 92;
        auto asymCol = center.removeFromRight(asymColW);
        asymCol.reduce(6, 2);
        labelAsymmetry.setJustificationType(juce::Justification::centred);
        readoutAsymmetry.setJustificationType(juce::Justification::centred);
        labelAsymmetry.setBounds(asymCol.removeFromTop(16));
        readoutAsymmetry.setBounds(asymCol.removeFromBottom(16));
        sliderAsymmetry.setBounds(asymCol.reduced(24, 2));

        // Now the remaining 'center' rect is the scope column. Lay out its header,
        // the rotation band underneath, and the scope fills what's left.
        sectionStereoImage.setBounds(center.removeFromTop(18));

        auto rotBand = center.removeFromBottom(28);
        rotBand.reduce(14, 4);
        labelRotation.setBounds(rotBand.removeFromLeft(78));
        readoutRotation.setBounds(rotBand.removeFromRight(56));
        sliderRotation.setBounds(rotBand.reduced(8, 4));

        scope->setBounds(center.reduced(10, 4));

        const int knobW = sideKnobW;
        const int totalW = knobW * 3 + 20 * 2;
        auto knobsBand = bottomRow.withSizeKeepingCentre(totalW, bottomRow.getHeight());

        auto shuffleCol = knobsBand.removeFromLeft(knobW);
        knobsBand.removeFromLeft(20);
        auto spaceCol = knobsBand.removeFromLeft(knobW);
        knobsBand.removeFromLeft(20);
        auto widthCol = knobsBand;

        auto layoutKnobCol = [](juce::Rectangle<int> col, juce::Label& title, juce::Label& unit, PourKnob& knob) {
            title.setBounds(col.removeFromTop(18));
            unit.setBounds(col.removeFromBottom(16));
            knob.setBounds(col.reduced(2, 2));
        };
        layoutKnobCol(shuffleCol, sectionShuffle, unitShuffleHz, knobShuffle);
        layoutKnobCol(spaceCol,   sectionSpace,   unitSpaceDb,   knobSpace);
        layoutKnobCol(widthCol,   sectionWidth,   unitWidthDb,   knobWidth);
    }
}

void PourPanel::timerCallback() {
    auto lv = proc.getEngine().getLevels();
    meterIn.setLevels(lv.inL, lv.inR);
    meterOut.setLevels(lv.outL, lv.outR);
    bottomBar.setCanUndo(!undoStack.empty());
    bottomBar.setCanRedo(!redoStack.empty());

    // Feed overlay values into the scope so the S1 triangle reflects current params.
    if (scope) {
        scope->setOverlay(
            (float) knobWidth.getValue(),
            (float) sliderRotation.getValue(),
            (float) sliderAsymmetry.getValue());
    }
}

void PourPanel::applyPreset(int idx) {
    pushUndo();
    presetStore.apply(idx, proc.getState());
    currentPresetIdx = idx;
    bottomBar.setPresetName(presetStore.all()[idx].name);
}

void PourPanel::pushUndo() {
    undoStack.push_back(proc.getState().copyState());
    if (undoStack.size() > 64) undoStack.pop_front();
    redoStack.clear();
}

void PourPanel::swapAB(char newSlot) {
    if (newSlot == abSlot) return;
    auto current = proc.getState().copyState();
    proc.getState().replaceState(otherSlotState.isValid() ? otherSlotState : current);
    otherSlotState = current;
    abSlot = newSlot;
    bottomBar.setAB(abSlot);
}

void PourPanel::openPresetBrowser(juce::Point<int> /*screenPt*/) {
    auto browser = std::make_unique<PresetBrowser>(presetStore, proc.getState(),
        [this](int idx) {
            applyPreset(idx);
            if (presetCallout) presetCallout->dismiss();
        });
    browser->setSize(280, 320);

    auto* comp = browser.release();
    auto anchorArea = bottomBar.getBounds();
    presetCallout.reset();
    auto& cb = juce::CallOutBox::launchAsynchronously(
        std::unique_ptr<juce::Component>(comp),
        localAreaToGlobal(anchorArea.removeFromTop(anchorArea.getHeight() / 2)
                          .withSizeKeepingCentre(200, 10)),
        nullptr);
    cb.setDismissalMouseClicksAreAlwaysConsumed(true);
}

} // namespace pour
