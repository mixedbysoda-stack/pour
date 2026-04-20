#pragma once
#include <juce_dsp/juce_dsp.h>

namespace pour {

// M/S stereo imager: Input Trim -> M/S split -> (Space low-shelf on Side, crossover = Shuffle)
// -> Width gain on Side -> M/S sum -> Solo Side tap -> Output Trim.
// Based on SSL Fusion Stereo Image / Blumlein Stereo Shuffling (1931).
class StereoImageEngine {
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Block-rate parameter updates (call before process each block).
    void setInputTrimDb(float db)  { inputTrimDb.setTargetValue(db); }
    void setOutputTrimDb(float db) { outputTrimDb.setTargetValue(db); }
    void setShuffleHz(float hz);
    void setSpaceDb(float db);
    void setWidthDb(float db)      { widthDb.setTargetValue(db); }
    void setRotationDeg(float d)   { rotationDeg.setTargetValue(d); }
    void setAsymmetryDeg(float d)  { asymmetryDeg.setTargetValue(d); }
    void setEnabled(bool v)        { enabled = v; }
    void setSoloSide(bool v)       { soloSide = v; }

    // Dry-bypass-aware process. In-place on buffer (first 2 channels).
    void process(juce::AudioBuffer<float>& buffer);

    // Feedback for UI (thread-safe reads from message thread).
    struct Levels { float inL = 0, inR = 0, outL = 0, outR = 0; };
    Levels getLevels() const noexcept { return levelsSnapshot.load(); }

    // Scope buffer: a ring of recent L/R samples at a downsampled rate.
    static constexpr int scopeSize = 2048;
    struct ScopeFrame { float l = 0, r = 0; };
    // Copies latest N samples into dst (newest last). Returns count copied.
    int readScope(ScopeFrame* dst, int maxCount) const noexcept;

private:
    double sr = 44100.0;
    int maxBlock = 512;

    // Parameter smoothers
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputTrimDb { 0.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputTrimDb { 0.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthDb { 0.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> spaceDb { 0.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> shuffleHz { 100.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> rotationDeg { 0.f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> asymmetryDeg { 0.f };

    bool enabled = true;
    bool soloSide = false;

    // Low-shelf applied to the side signal (Space parameter). Updated per-block.
    using Filter = juce::dsp::IIR::Filter<float>;
    Filter sideShelf;
    float lastShelfHz = -1.f, lastShelfDb = -999.f;

    void updateSideShelf(float hz, float gainDb);

    // Level snapshots for UI
    std::atomic<Levels> levelsSnapshot { Levels{} };

    // Scope ring buffer (written from audio thread, read by UI)
    mutable std::array<ScopeFrame, scopeSize> scopeRing {};
    std::atomic<int> scopeWrite { 0 };
    int scopeDecim = 8;  // store 1 in N samples
    int scopeCounter = 0;
};

} // namespace pour
