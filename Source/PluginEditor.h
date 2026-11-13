#pragma once
#include <JuceHeader.h>
#include <cstdint>
#include "PluginProcessor.h"
#include "ui/Controls.h"
#include "ui/LookAndFeel.h"
#include <SkellCore/WorldLink.h>
class SkellyDeEsserAudioProcessorEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit SkellyDeEsserAudioProcessorEditor(SkellyDeEsserAudioProcessor&);
 ~SkellyDeEsserAudioProcessorEditor() override;
 void paint(juce::Graphics&)override;
 void resized()override;
private:
 void timerCallback()override;
 void showSettings();
 void savePresetFile();
 void loadPresetFile();
 void updateButtons();
 void setChoice(const char*,int);
 void setFloat(const char*,float);
 int getDetectorFilterMode() const;
 void applyDetectorFilterMode(int);
 SkellyDeEsserAudioProcessor& processor;
 SkellyLookAndFeel look;
 // Explicit UI-only opt-in; no audio callback or saved APVTS state changes.
 skellcore::world::WorldLink worldLink{"skell.deesser", "Skelly DeEsser"};
 SkullKnob frequency;
 ThresholdFader threshold;
 ReadoutBox frequencyValue,thresholdValue;
 TwoStateButton split{"Split"},wide{"Wide"},phraseButton{"Phrase"},audio{"Audio"},sidechain{"S Chain"};
 IconButton hpf{"HP"},filter{"FILTER"};
 IconButton prev{"PREV"},next{"NEXT"},save{"SAVE"},settings{"MENU"},preset{"A: My Default Preset"};
 SectionHeader detectHeader{"DETECTION"},frequencyHeader{"FREQUENCY"},sideHeader{"SIDECHAIN"},monitorHeader{"MONITOR"};
 WaveformBackdrop waveform;
 juce::Image artFrame,stageArt,leftArt,faderArt,capArt,knobArt,staticBackdrop;
 juce::ComponentBoundsConstrainer aspect;
 // UI-only paint revision cache. No locks or audio-thread access to GUI.
 uint64_t lastWaveSerial=UINT64_MAX;
 float lastSideDb=999.f,lastGrDb=999.f,lastPhraseDb=999.f;
 int lastLearnedEvents=-1;
 bool lastPhraseOn=false;
 std::unique_ptr<juce::FileChooser> presetChooser;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttach,thresholdAttach;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellyDeEsserAudioProcessorEditor)
};
