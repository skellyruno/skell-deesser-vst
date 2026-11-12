cmake_minimum_required(VERSION 3.22)
project(SkellyDeEsser VERSION 1.16.0 LANGUAGES C CXX)
include(FetchContent)
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
  add_subdirectory("${JUCE_PATH}" JUCE)
else()
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(SkellyDeEsser COMPANY_NAME "Skelly Audio" BUNDLE_ID "audio.skelly.deesser"
 PLUGIN_MANUFACTURER_CODE SkAu PLUGIN_CODE SkDs FORMATS VST3
 PRODUCT_NAME "Skelly DeEsser" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(SkellyDeEsser)
juce_add_binary_data(SkellyArt SOURCES
 Assets/frame.png Assets/skull_emblem.png Assets/stage_bg.png Assets/left_panel_bg.png
 Assets/knob_skull.png Assets/fader_cap.png Assets/fader_housing.png)
target_sources(SkellyDeEsser PRIVATE
 Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h
 Source/ui/Theme.h Source/ui/LookAndFeel.h Source/ui/Controls.h
 Source/dsp/WaveformHistory.h Source/dsp/EnvelopeFollower.h Source/dsp/SidechainFilter.h Source/dsp/ComplementaryBand.h Source/dsp/PhraseAwareDetector.h Source/dsp/DeEsserEngine.h)
target_compile_features(SkellyDeEsser PRIVATE cxx_std_17)
target_compile_definitions(SkellyDeEsser PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SkellyDeEsser PRIVATE SkellyArt juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra)


# Development-only real-JUCE-editor screenshot harness. The shipped VST3
# does not link SkellCore or carry renderer code. Pin upstream, never track main.
option(SKELLY_DEESSER_BUILD_UI_RENDERER "Build real editor SkellCore PNG capture tool" OFF)
if(SKELLY_DEESSER_BUILD_UI_RENDERER)
  if(DEFINED SKELLCORE_PATH AND EXISTS "${SKELLCORE_PATH}/CMakeLists.txt")
    add_subdirectory("${SKELLCORE_PATH}" SkellCoreLocal)
  else()
    FetchContent_Declare(SkellCoreRendererDependency
      GIT_REPOSITORY https://github.com/skellyruno/skell-vst-template.git
      GIT_TAG 438bc74a86a40bf54cfa8089832a477822c8ccda
      GIT_SHALLOW FALSE
      SOURCE_SUBDIR SkellCore)
    FetchContent_MakeAvailable(SkellCoreRendererDependency)
  endif()

  if(NOT TARGET SkellCore::UIRenderer)
    message(FATAL_ERROR "Pinned SkellCore commit does not provide SkellCore::UIRenderer")
  endif()
  # Plain executable, not juce_add_console_app: that macro's
  # JUCE_STANDALONE_APPLICATION definition conflicts with plugin SharedCode.
  add_executable(SkellyDeEsserUIRenderer tests/ui_renderer.cpp)
  target_include_directories(SkellyDeEsserUIRenderer PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/SkellyDeEsser_artefacts/JuceLibraryCode")
  target_link_libraries(SkellyDeEsserUIRenderer PRIVATE
    SkellyDeEsser SkellyArt SkellCore::UIRenderer
    juce::juce_audio_utils juce::juce_gui_extra)
  target_compile_definitions(SkellyDeEsserUIRenderer PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  target_compile_features(SkellyDeEsserUIRenderer PRIVATE cxx_std_17)
endif()
