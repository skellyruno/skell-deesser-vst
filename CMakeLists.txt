cmake_minimum_required(VERSION 3.22)
project(SkellyDeEsser VERSION 1.2.0 LANGUAGES C CXX)
include(FetchContent)
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
  add_subdirectory("${JUCE_PATH}" JUCE)
else()
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(SkellyDeEsser COMPANY_NAME "Skelly Audio" BUNDLE_ID "audio.skelly.deesser"
 PLUGIN_MANUFACTURER_CODE SkAu PLUGIN_CODE SkDs FORMATS VST3 Standalone
 PRODUCT_NAME "Skelly DeEsser" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(SkellyDeEsser)
juce_add_binary_data(SkellyArt SOURCES
 Assets/frame.png Assets/skull_emblem.png Assets/stage_bg.png Assets/left_panel_bg.png
 Assets/knob_skull.png Assets/fader_cap.png Assets/fader_housing.png)
target_sources(SkellyDeEsser PRIVATE
 Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h
 Source/ui/Theme.h Source/ui/LookAndFeel.h Source/ui/Controls.h
 Source/dsp/WaveformHistory.h Source/dsp/EnvelopeFollower.h Source/dsp/SidechainFilter.h Source/dsp/DeEsserEngine.h)
target_compile_features(SkellyDeEsser PRIVATE cxx_std_17)
target_compile_definitions(SkellyDeEsser PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SkellyDeEsser PRIVATE SkellyArt juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra)
