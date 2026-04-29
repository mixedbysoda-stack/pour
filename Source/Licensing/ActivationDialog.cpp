#include "ActivationDialog.h"

ActivationDialog::ActivationDialog(LicenseManager& lm)
    : licenseManager(lm),
      buyLink ("Buy Pour",    juce::URL("https://carbonatedaudio.com/pour")),
      helpLink("Need help?",  juce::URL("mailto:hello@carbonatedaudio.com"))
{
    setInterceptsMouseClicks(true, true);

    titleLabel.setText("Activate Pour", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    instructionLabel.setText("Enter the license key from your purchase confirmation.",
                             juce::dontSendNotification);
    instructionLabel.setFont(juce::FontOptions(13.0f));
    instructionLabel.setJustificationType(juce::Justification::centred);
    instructionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    addAndMakeVisible(instructionLabel);

    keyInput.setMultiLine(false);
    keyInput.setReturnKeyStartsNewLine(false);
    keyInput.setFont(juce::FontOptions(13.0f));
    keyInput.setTextToShowWhenEmpty("XXXXXXXX-XXXXXXXX-XXXXXXXX-...", juce::Colour(0xff5a6068));
    keyInput.setJustification(juce::Justification::centred);
    keyInput.setColour(juce::TextEditor::backgroundColourId,      juce::Colour(0xff0a0b0d));
    keyInput.setColour(juce::TextEditor::outlineColourId,         juce::Colour(0xff2e3135));
    keyInput.setColour(juce::TextEditor::textColourId,            juce::Colour(0xff4fb8d4));
    keyInput.setColour(juce::TextEditor::focusedOutlineColourId,  juce::Colour(0xff4fb8d4));
    keyInput.onReturnKey = [this]() { onActivateClicked(); };
    addAndMakeVisible(keyInput);

    activateButton.setButtonText("Activate");
    activateButton.onClick = [this]() { onActivateClicked(); };
    activateButton.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff4fb8d4));
    activateButton.setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
    activateButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(activateButton);

    statusLabel.setText("", juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(13.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    buyLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff4fb8d4));
    buyLink.setFont(juce::FontOptions(13.0f), false);
    addAndMakeVisible(buyLink);

    helpLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff9aa0a6));
    helpLink.setFont(juce::FontOptions(13.0f), false);
    addAndMakeVisible(helpLink);
}

void ActivationDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xee0a0b0d));

    auto cardBounds = getLocalBounds().toFloat().withSizeKeepingCentre(400.0f, 360.0f);

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(cardBounds.translated(0.0f, 4.0f), 16.0f);

    g.setColour(juce::Colour(0xff1a1c1f));
    g.fillRoundedRectangle(cardBounds, 16.0f);

    g.setColour(juce::Colour(0xff2e3135));
    g.drawRoundedRectangle(cardBounds, 16.0f, 1.0f);
}

void ActivationDialog::resized()
{
    auto cardBounds = getLocalBounds().withSizeKeepingCentre(400, 360);
    auto area = cardBounds.reduced(28);

    titleLabel.setBounds(area.removeFromTop(32));
    area.removeFromTop(8);

    instructionLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(20);

    keyInput.setBounds(area.removeFromTop(36));
    area.removeFromTop(16);

    activateButton.setBounds(area.removeFromTop(40).reduced(80, 0));
    area.removeFromTop(12);

    statusLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(16);

    auto linkArea = area.removeFromTop(20);
    auto halfW = linkArea.getWidth() / 2;
    buyLink.setBounds(linkArea.removeFromLeft(halfW));
    helpLink.setBounds(linkArea);
}

void ActivationDialog::visibilityChanged()
{
    if (! isVisible())
        return;

    // Defer the focus grab one message-loop tick so it never runs while the
    // editor is still mid-construction (zero bounds, no peer). Calling
    // grabKeyboardFocus() synchronously from visibilityChanged() crashed
    // Cubase 15 / Win 10 on JUCE 8's GDI software renderer.
    juce::Component::SafePointer<juce::TextEditor> safeInput (&keyInput);
    juce::MessageManager::callAsync ([safeInput]()
    {
        if (safeInput != nullptr && safeInput->isShowing())
            safeInput->grabKeyboardFocus();
    });
}

void ActivationDialog::onActivateClicked()
{
    auto key = keyInput.getText().trim();

    if (key.isEmpty())
    {
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffef4444));
        statusLabel.setText("Please enter your license key", juce::dontSendNotification);
        return;
    }

    activateButton.setEnabled(false);
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    statusLabel.setText("Activating...", juce::dontSendNotification);

    auto safeThis = juce::Component::SafePointer<ActivationDialog>(this);

    licenseManager.tryActivate(key, [safeThis](bool success, const juce::String& message)
    {
        if (safeThis == nullptr)
            return;

        if (success)
        {
            safeThis->statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff10b981));
            safeThis->statusLabel.setText(message, juce::dontSendNotification);
        }
        else
        {
            safeThis->statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffef4444));
            safeThis->statusLabel.setText(message, juce::dontSendNotification);
            safeThis->activateButton.setEnabled(true);
        }
    });
}
