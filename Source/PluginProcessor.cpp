#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pour {

PourProcessor::PourProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Pour", params::createLayout())
{
    licenseManager = std::make_unique<LicenseManager>();
    stateA = apvts.copyState();
    stateB = apvts.copyState();
}

void PourProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    engine.prepare(sampleRate, samplesPerBlock);
#if POUR_DEMO
    demoSampleRate = sampleRate;
    demoSampleCounter = 0;
#endif
}

void PourProcessor::releaseResources() {
    engine.reset();
}

bool PourProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in.isDisabled() || out.isDisabled()) return false;
    return in == out && (in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::mono());
}

void PourProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

#if POUR_DEMO
    // Demo gate: 60 s play, 10 s silence, repeating — mirrors De-Sipper.
    {
        const int n = buffer.getNumSamples();
        const juce::int64 cycle = (juce::int64) (70.0 * demoSampleRate);
        const juce::int64 muteAt = (juce::int64) (60.0 * demoSampleRate);
        const juce::int64 pos = demoSampleCounter % cycle;
        demoSampleCounter += n;
        if (pos >= muteAt) {
            buffer.clear();
            return;
        }
    }
#else
    // License gate: silence audio if the plug-in is not activated.
    if (licenseManager && !licenseManager->isActivated()) {
        buffer.clear();
        return;
    }
#endif

    // Push parameter values into engine
    engine.setInputTrimDb (apvts.getRawParameterValue(params::inputTrim)->load());
    engine.setOutputTrimDb(apvts.getRawParameterValue(params::outputTrim)->load());
    engine.setShuffleHz   (apvts.getRawParameterValue(params::shuffle)->load());
    engine.setSpaceDb     (apvts.getRawParameterValue(params::space)->load());
    engine.setWidthDb     (apvts.getRawParameterValue(params::width)->load());
    engine.setRotationDeg (apvts.getRawParameterValue(params::rotation)->load());
    engine.setAsymmetryDeg(apvts.getRawParameterValue(params::asymmetry)->load());
    engine.setEnabled     (apvts.getRawParameterValue(params::inEnabled)->load() > 0.5f);
    engine.setSoloSide    (apvts.getRawParameterValue(params::soloSide)->load() > 0.5f);

    // Mono input: duplicate to stereo so the stereo imager has something to work on.
    if (buffer.getNumChannels() == 1) return;

    engine.process(buffer);
}

juce::AudioProcessorEditor* PourProcessor::createEditor() {
    return new PourEditor(*this);
}

void PourProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ValueTree root("PourState");
    root.setProperty("abSlot", juce::String::charToString((juce::juce_wchar) abSlot), nullptr);
    root.appendChild(apvts.copyState(), nullptr);
    root.appendChild(stateA.createCopy().hasType("A") ? stateA : stateA.createCopy(), nullptr);
    // Simpler: serialize as two distinct named children
    juce::ValueTree a("A"); a.copyPropertiesAndChildrenFrom(stateA, nullptr);
    juce::ValueTree b("B"); b.copyPropertiesAndChildrenFrom(stateB, nullptr);
    root.removeAllChildren(nullptr);
    auto live = apvts.copyState();
    root.appendChild(live, nullptr);
    root.appendChild(a, nullptr);
    root.appendChild(b, nullptr);
    juce::MemoryOutputStream mos(destData, false);
    root.writeToStream(mos);
}

void PourProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto root = juce::ValueTree::readFromData(data, (size_t) sizeInBytes);
    if (!root.isValid()) return;
    if (auto slotStr = root.getProperty("abSlot").toString(); slotStr.isNotEmpty())
        abSlot = (char) slotStr[0];
    for (int i = 0; i < root.getNumChildren(); ++i) {
        auto child = root.getChild(i);
        if (child.hasType("A")) { stateA = juce::ValueTree("Pour"); stateA.copyPropertiesAndChildrenFrom(child, nullptr); }
        else if (child.hasType("B")) { stateB = juce::ValueTree("Pour"); stateB.copyPropertiesAndChildrenFrom(child, nullptr); }
        else if (child.hasType("Pour")) apvts.replaceState(child);
    }
}

} // namespace pour

// Factory function JUCE expects
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new pour::PourProcessor();
}
