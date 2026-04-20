#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace pour::params {
    // Parameter IDs
    static constexpr auto inputTrim  = "inputTrim";
    static constexpr auto outputTrim = "outputTrim";
    static constexpr auto shuffle    = "shuffle";
    static constexpr auto space      = "space";
    static constexpr auto width      = "width";
    static constexpr auto inEnabled  = "inEnabled";
    static constexpr auto soloSide   = "soloSide";
    static constexpr auto rotation   = "rotation";
    static constexpr auto asymmetry  = "asymmetry";

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
