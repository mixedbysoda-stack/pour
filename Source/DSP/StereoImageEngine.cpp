#include "StereoImageEngine.h"

namespace pour {

static inline float dbToGain(float db) {
    return std::pow(10.0f, db / 20.0f);
}

void StereoImageEngine::prepare(double sampleRate, int maxBlockSize) {
    sr = sampleRate;
    maxBlock = maxBlockSize;

    const double smoothTime = 0.02; // 20 ms
    inputTrimDb.reset(sampleRate, smoothTime);
    outputTrimDb.reset(sampleRate, smoothTime);
    widthDb.reset(sampleRate, smoothTime);
    spaceDb.reset(sampleRate, smoothTime);
    shuffleHz.reset(sampleRate, 0.05);
    rotationDeg.reset(sampleRate, smoothTime);
    asymmetryDeg.reset(sampleRate, smoothTime);

    sideShelf.reset();
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
    sideShelf.prepare(spec);
    lastShelfHz = lastShelfDb = -999.f;
    updateSideShelf(shuffleHz.getCurrentValue(), spaceDb.getCurrentValue());

    scopeWrite = 0;
    scopeCounter = 0;
    for (auto& f : scopeRing) f = {};
}

void StereoImageEngine::reset() {
    sideShelf.reset();
    scopeWrite = 0;
    scopeCounter = 0;
    for (auto& f : scopeRing) f = {};
}

void StereoImageEngine::setShuffleHz(float hz) {
    shuffleHz.setTargetValue(juce::jlimit(20.0f, 500.0f, hz));
}

void StereoImageEngine::setSpaceDb(float db) {
    spaceDb.setTargetValue(juce::jlimit(-24.0f, 24.0f, db));
}

void StereoImageEngine::updateSideShelf(float hz, float gainDb) {
    if (std::abs(hz - lastShelfHz) < 0.5f && std::abs(gainDb - lastShelfDb) < 0.01f) return;
    lastShelfHz = hz;
    lastShelfDb = gainDb;
    const float gain = dbToGain(gainDb);
    // Q ~ 0.707 for a gentle broad shelf, matches the "broad bass boost/cut" spec.
    *sideShelf.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr, hz, 0.707f, gain);
}

void StereoImageEngine::process(juce::AudioBuffer<float>& buffer) {
    const int n = buffer.getNumSamples();
    if (n == 0 || buffer.getNumChannels() < 2) return;

    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getWritePointer(1);

    // Measure input peaks
    float peakInL = 0.f, peakInR = 0.f;
    for (int i = 0; i < n; ++i) {
        peakInL = std::max(peakInL, std::abs(L[i]));
        peakInR = std::max(peakInR, std::abs(R[i]));
    }

    // Update filter from smoothed params at block start
    updateSideShelf(shuffleHz.getNextValue(), spaceDb.getNextValue());

    float peakOutL = 0.f, peakOutR = 0.f;

    if (!enabled) {
        // Bypassed: pass through, still push to scope so it shows signal.
        for (int i = 0; i < n; ++i) {
            peakOutL = std::max(peakOutL, std::abs(L[i]));
            peakOutR = std::max(peakOutR, std::abs(R[i]));
            if (++scopeCounter >= scopeDecim) {
                scopeCounter = 0;
                int w = scopeWrite.load(std::memory_order_relaxed);
                scopeRing[w] = { L[i], R[i] };
                scopeWrite.store((w + 1) % scopeSize, std::memory_order_release);
            }
        }
        levelInL.store(peakInL,  std::memory_order_relaxed);
        levelInR.store(peakInR,  std::memory_order_relaxed);
        levelOutL.store(peakOutL, std::memory_order_relaxed);
        levelOutR.store(peakOutR, std::memory_order_relaxed);
        return;
    }

    // Per-sample processing (sample-accurate smoothing)
    constexpr float degToRad = 0.01745329252f;
    for (int i = 0; i < n; ++i) {
        const float inGain  = dbToGain(inputTrimDb.getNextValue());
        const float outGain = dbToGain(outputTrimDb.getNextValue());
        const float wGain   = dbToGain(widthDb.getNextValue());
        const float rotRad  = rotationDeg.getNextValue() * degToRad;
        const float asymN   = asymmetryDeg.getNextValue() / 90.0f; // -1..+1

        float l = L[i] * inGain;
        float r = R[i] * inGain;

        // Encode to M/S
        float m = 0.5f * (l + r);
        float s = 0.5f * (l - r);

        // Space: apply low-shelf on side (Stereo Shuffling bass shelf)
        s = sideShelf.processSample(s);

        // Width: gain on side
        s *= wGain;

        // Rotation: rotate the M/S vector (tilts the stereo image left/right).
        if (std::abs(rotRad) > 1.0e-5f) {
            const float cR = std::cos(rotRad);
            const float sR = std::sin(rotRad);
            const float nm = m * cR - s * sR;
            const float ns = m * sR + s * cR;
            m = nm; s = ns;
        }

        float outL, outR;
        if (soloSide) {
            outL = outR = s;
        } else {
            outL = (m + s);
            outR = (m - s);
        }

        // Asymmetry: constant-power pan on output. Shifts the phantom center without affecting width.
        if (std::abs(asymN) > 1.0e-5f) {
            const float t = (asymN * 0.5f + 0.5f) * 1.5707963268f; // 0..π/2
            const float panL = std::cos(t) * 1.41421356f; // at 0°, cos(π/4)*√2 = 1
            const float panR = std::sin(t) * 1.41421356f;
            outL *= panL;
            outR *= panR;
        }

        outL *= outGain;
        outR *= outGain;

        L[i] = outL;
        R[i] = outR;

        peakOutL = std::max(peakOutL, std::abs(outL));
        peakOutR = std::max(peakOutR, std::abs(outR));

        if (++scopeCounter >= scopeDecim) {
            scopeCounter = 0;
            int w = scopeWrite.load(std::memory_order_relaxed);
            scopeRing[w] = { outL, outR };
            scopeWrite.store((w + 1) % scopeSize, std::memory_order_release);
        }
    }

    levelInL.store(peakInL,  std::memory_order_relaxed);
    levelInR.store(peakInR,  std::memory_order_relaxed);
    levelOutL.store(peakOutL, std::memory_order_relaxed);
    levelOutR.store(peakOutR, std::memory_order_relaxed);
}

int StereoImageEngine::readScope(ScopeFrame* dst, int maxCount) const noexcept {
    const int w = scopeWrite.load(std::memory_order_acquire);
    const int count = std::min(maxCount, scopeSize);
    int start = (w - count + scopeSize) % scopeSize;
    for (int i = 0; i < count; ++i) {
        dst[i] = scopeRing[(start + i) % scopeSize];
    }
    return count;
}

} // namespace pour
