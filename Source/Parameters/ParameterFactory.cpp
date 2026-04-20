#include "ParameterFactory.h"

namespace pour::params {

using APVTS = juce::AudioProcessorValueTreeState;
using RangedParam = juce::RangedAudioParameter;

static juce::String dbString(float v, int) {
    if (std::abs(v) < 0.05f) return "0.0 dB";
    return (v >= 0.f ? "+" : "") + juce::String(v, 1) + " dB";
}
static juce::String hzString(float v, int) {
    return juce::String(juce::roundToInt(v)) + " Hz";
}
static juce::String degString(float v, int) {
    return juce::String(v, 1) + juce::String::charToString((juce::juce_wchar) 0x00B0);
}

APVTS::ParameterLayout createLayout() {
    std::vector<std::unique_ptr<RangedParam>> p;

    // Input Trim: -12..+12 dB, 0 default
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { inputTrim, 1 }, "Input Trim",
        juce::NormalisableRange<float> { -12.0f, 12.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(dbString).withLabel("dB")));

    // Output Trim: -12..+12 dB, 0 default
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { outputTrim, 1 }, "Output Trim",
        juce::NormalisableRange<float> { -12.0f, 12.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(dbString).withLabel("dB")));

    // Shuffle: 40..400 Hz, logarithmic-ish (skew so 100Hz sits around 25% of knob)
    auto shuffleRange = juce::NormalisableRange<float> { 40.0f, 400.0f, 1.0f };
    shuffleRange.setSkewForCentre(160.0f);
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { shuffle, 1 }, "Shuffle",
        shuffleRange, 100.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(hzString).withLabel("Hz")));

    // Space: -12..+12 dB, 0 default (side-signal bass shelf)
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { space, 1 }, "Space",
        juce::NormalisableRange<float> { -12.0f, 12.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(dbString).withLabel("dB")));

    // Width: -6..+6 dB, 0 default (side-signal gain)
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { width, 1 }, "Width",
        juce::NormalisableRange<float> { -6.0f, 6.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(dbString).withLabel("dB")));

    // IN (plug-in active). Default true.
    p.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { inEnabled, 1 }, "In", true));

    // Solo Side. Default false.
    p.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { soloSide, 1 }, "Solo Side", false));

    // Rotation: -45..+45 degrees, 0 default. Tilts the stereo image.
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { rotation, 1 }, "Rotation",
        juce::NormalisableRange<float> { -45.0f, 45.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(degString).withLabel(juce::String::charToString((juce::juce_wchar) 0x00B0))));

    // Asymmetry: -90..+90 degrees, 0 default. Shifts the phantom center (balance on M).
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { asymmetry, 1 }, "Asymmetry",
        juce::NormalisableRange<float> { -90.0f, 90.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(degString).withLabel(juce::String::charToString((juce::juce_wchar) 0x00B0))));

    return { p.begin(), p.end() };
}

} // namespace pour::params
