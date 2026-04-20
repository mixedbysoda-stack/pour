cmake_minimum_required(VERSION 3.22)
project(Pour VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

add_subdirectory(JUCE)

juce_add_plugin(Pour
    COMPANY_NAME "Carbonated Audio"
    PLUGIN_MANUFACTURER_CODE CbAu
    PLUGIN_CODE Pour
    FORMATS VST3 AU AAX Standalone
    PRODUCT_NAME "Pour"
    BUNDLE_ID "com.carbonatedaudio.pour"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx" "Spatial" "Stereo"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    AAX_CATEGORY "AAX_ePlugInCategory_Modulation"
)

target_sources(Pour PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters/ParameterFactory.cpp
    Source/DSP/StereoImageEngine.cpp
    Source/Licensing/LicenseManager.cpp
    Source/Licensing/ActivationDialog.cpp
    Source/UI/PourPanel.cpp
    Source/UI/LookAndFeel/PourLookAndFeel.cpp
    Source/UI/Components/PourKnob.cpp
    Source/UI/Components/LedMeter.cpp
    Source/UI/Components/Goniometer.cpp
    Source/UI/Components/PourButton.cpp
    Source/UI/Components/BottomBar.cpp
    Source/UI/Components/PresetBrowser.cpp
)

juce_add_binary_data(PourBinaryData SOURCES
    Resources/CarbonatedLogo.png
)

target_link_libraries(Pour PRIVATE PourBinaryData)

target_compile_definitions(Pour PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(Pour PRIVATE
    juce::juce_audio_utils
    juce::juce_cryptography
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(APPLE)
    target_link_libraries(Pour PRIVATE "-framework IOKit")
endif()
