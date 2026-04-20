#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace pour {

struct PresetEntry {
    juce::String name;
    juce::String category;
    std::map<juce::String, float> values;  // paramID -> value (dB/Hz/bool-as-0-or-1)
};

class PresetStore {
public:
    PresetStore();
    const std::vector<PresetEntry>& all() const noexcept { return presets; }
    int findIndex(const juce::String& name) const;
    void apply(int index, juce::AudioProcessorValueTreeState& apvts) const;

    std::vector<juce::String> categories() const;
    std::vector<int> indicesForCategory(const juce::String& cat) const;

private:
    std::vector<PresetEntry> presets;
};

// Pop-up preset picker modelled on the SSL Fusion preset menu.
// Categories: Default, Bass, Vocal, Drums, Guitar (+ user).
class PresetBrowser : public juce::Component {
public:
    PresetBrowser(PresetStore& store,
                  juce::AudioProcessorValueTreeState& apvts,
                  std::function<void(int)> onPick);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Row : juce::Component {
        Row(juce::String l, bool hdr, std::function<void()> click)
            : label(std::move(l)), header(hdr), onClick(std::move(click)) {}
        void paint(juce::Graphics&) override;
        void mouseUp(const juce::MouseEvent&) override { if (!header && isEnabled() && onClick) onClick(); }
        void mouseEnter(const juce::MouseEvent&) override { if (!header) { hover = true; repaint(); } }
        void mouseExit(const juce::MouseEvent&) override { hover = false; repaint(); }
        juce::String label;
        bool header;
        bool hover = false;
        std::function<void()> onClick;
    };

    PresetStore& store;
    juce::AudioProcessorValueTreeState& apvts;
    std::function<void(int)> onPick;

    juce::Viewport viewport;
    juce::Component list;
    std::vector<std::unique_ptr<Row>> rows;

    void buildRows();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};

} // namespace pour
