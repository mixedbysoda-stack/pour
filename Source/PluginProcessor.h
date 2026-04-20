#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/StereoImageEngine.h"
#include "Parameters/ParameterFactory.h"
#include "Licensing/LicenseManager.h"

namespace pour {

class PourProcessor : public juce::AudioProcessor {
public:
    PourProcessor();
    ~PourProcessor() override = default;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Pour"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Access for editor
    juce::AudioProcessorValueTreeState& getState() noexcept { return apvts; }
    StereoImageEngine& getEngine() noexcept { return engine; }
    LicenseManager& getLicenseManager() noexcept { return *licenseManager; }

    // A/B compare state lives here so it survives editor open/close.
    juce::ValueTree getABState(char slot) const { return slot == 'A' ? stateA : stateB; }
    void setABState(char slot, juce::ValueTree v) { (slot == 'A' ? stateA : stateB) = std::move(v); }

    char getABSlot() const noexcept { return abSlot; }
    void setABSlot(char s) noexcept { abSlot = s; }

private:
    juce::AudioProcessorValueTreeState apvts;
    StereoImageEngine engine;
    std::unique_ptr<LicenseManager> licenseManager;

    // A/B parameter snapshots
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    char abSlot = 'A';

#if POUR_DEMO
    // Demo audio gate: 60s pass, 10s silence, repeating.
    juce::int64 demoSampleCounter = 0;
    double demoSampleRate = 44100.0;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PourProcessor)
};

} // namespace pour
