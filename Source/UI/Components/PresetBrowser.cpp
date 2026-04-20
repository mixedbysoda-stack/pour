#include "PresetBrowser.h"
#include "../LookAndFeel/PourLookAndFeel.h"
#include "../../Parameters/ParameterFactory.h"

namespace pour {

PresetStore::PresetStore() {
    using P = PresetEntry;
    // name, category, param map. Shuffle Hz, Space dB, Width dB (Input/Output trim kept at 0 by default).
    presets = {
        P{ "Default Preset",         "Default", {{ params::shuffle,100.f},{ params::space, 0.f},{ params::width, 0.f}} },
        P{ "Tight Low End",          "Bass",    {{ params::shuffle, 60.f},{ params::space,-4.f},{ params::width,-1.5f}} },
        P{ "Wide Sub",               "Bass",    {{ params::shuffle, 80.f},{ params::space, 6.0f},{ params::width, 2.0f}} },
        P{ "Intimate Vocal",         "Vocal",   {{ params::shuffle,180.f},{ params::space,-1.5f},{ params::width,-1.5f}} },
        P{ "Open Vocal",             "Vocal",   {{ params::shuffle,220.f},{ params::space, 2.5f},{ params::width, 3.0f}} },
        P{ "Punchy Drum Bus",        "Drums",   {{ params::shuffle,100.f},{ params::space, 1.5f},{ params::width, 2.0f}} },
        P{ "Wide Room",              "Drums",   {{ params::shuffle,280.f},{ params::space, 4.0f},{ params::width, 4.0f}} },
        P{ "Doubled Guitars",        "Guitar",  {{ params::shuffle,160.f},{ params::space, 1.0f},{ params::width, 3.5f}} },
        P{ "Stereo Spread Guitar",   "Guitar",  {{ params::shuffle,340.f},{ params::space, 3.0f},{ params::width, 4.5f}} },
    };
}

int PresetStore::findIndex(const juce::String& name) const {
    for (int i = 0; i < (int) presets.size(); ++i)
        if (presets[i].name == name) return i;
    return -1;
}

void PresetStore::apply(int index, juce::AudioProcessorValueTreeState& apvts) const {
    if (index < 0 || index >= (int) presets.size()) return;
    for (const auto& kv : presets[index].values) {
        if (auto* p = apvts.getParameter(kv.first)) {
            p->beginChangeGesture();
            p->setValueNotifyingHost(p->convertTo0to1(kv.second));
            p->endChangeGesture();
        }
    }
}

std::vector<juce::String> PresetStore::categories() const {
    std::vector<juce::String> out;
    for (const auto& p : presets)
        if (std::find(out.begin(), out.end(), p.category) == out.end())
            out.push_back(p.category);
    return out;
}

std::vector<int> PresetStore::indicesForCategory(const juce::String& cat) const {
    std::vector<int> out;
    for (int i = 0; i < (int) presets.size(); ++i)
        if (presets[i].category == cat) out.push_back(i);
    return out;
}

// ========== PresetBrowser ==========

void PresetBrowser::Row::paint(juce::Graphics& g) {
    if (header) {
        g.setColour(Colors::textSecondary);
        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 10.5f, juce::Font::bold));
        g.drawText(label.toUpperCase(), getLocalBounds().reduced(14, 0),
                   juce::Justification::centredLeft, false);
    } else {
        if (hover) {
            g.setColour(Colors::cyan.withAlpha(0.15f));
            g.fillRoundedRectangle(getLocalBounds().reduced(4, 2).toFloat(), 3.0f);
            g.setColour(Colors::cyanBright);
        } else {
            g.setColour(Colors::textPrimary);
        }
        g.setFont(12.5f);
        g.drawText(label, getLocalBounds().reduced(20, 0), juce::Justification::centredLeft, false);
    }
}

PresetBrowser::PresetBrowser(PresetStore& s,
                             juce::AudioProcessorValueTreeState& apvts_,
                             std::function<void(int)> pick)
    : store(s), apvts(apvts_), onPick(std::move(pick))
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&list, false);
    viewport.setScrollBarsShown(true, false);
    buildRows();
}

void PresetBrowser::buildRows() {
    rows.clear();
    auto cats = store.categories();
    int y = 8;
    for (const auto& c : cats) {
        auto hdr = std::make_unique<Row>(c, true, nullptr);
        hdr->setBounds(0, y, 260, 20);
        list.addAndMakeVisible(*hdr);
        rows.push_back(std::move(hdr));
        y += 22;
        for (int idx : store.indicesForCategory(c)) {
            const auto& name = store.all()[idx].name;
            auto r = std::make_unique<Row>(name, false,
                [this, idx]() {
                    store.apply(idx, apvts);
                    if (onPick) onPick(idx);
                });
            r->setBounds(0, y, 260, 26);
            list.addAndMakeVisible(*r);
            rows.push_back(std::move(r));
            y += 26;
        }
        y += 6;
    }
    list.setSize(260, y + 8);
}

void PresetBrowser::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour::fromRGB(18, 20, 23));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(Colors::cyan.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Header strip
    auto header = juce::Rectangle<float>(bounds.getX(), bounds.getY(), bounds.getWidth(), 32.0f).reduced(0.5f);
    g.setColour(juce::Colour::fromRGB(26, 29, 33));
    g.fillRoundedRectangle(header, 6.0f);
    g.setColour(Colors::textPrimary);
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 11.0f, juce::Font::bold));
    g.drawText("PRESETS", header.reduced(14, 0), juce::Justification::centredLeft, false);
}

void PresetBrowser::resized() {
    auto b = getLocalBounds().reduced(4);
    b.removeFromTop(28);
    viewport.setBounds(b);
}

} // namespace pour
