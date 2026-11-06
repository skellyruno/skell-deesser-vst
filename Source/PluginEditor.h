#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/Controls.h"
#include "ui/LookAndFeel.h"
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
 SkellyDeEsserAudioProcessor& processor;
 SkellyLookAndFeel look;
 SkullKnob frequency;
 ThresholdFader threshold;
 ReadoutBox frequencyValue,thresholdValue;
 TwoStateButton split{"Split"},wide{"Wide"},phraseButton{"Phrase"},audio{"Audio"},sidechain{"S Chain"};
 IconButton hpf{"HPF"},filter{"FILTER"};
 IconButton prev{"PREV"},next{"NEXT"},save{"SAVE"},settings{"MENU"},preset{"A: My Default Preset"};
 SectionHeader detectHeader{"DETECTION"},frequencyHeader{"FREQUENCY"},sideHeader{"SIDECHAIN"},monitorHeader{"MONITOR"};
 WaveformBackdrop waveform;
 juce::Image artFrame,stageArt,leftArt,faderArt,capArt,knobArt,staticBackdrop;
 juce::ComponentBoundsConstrainer aspect;
 std::unique_ptr<juce::FileChooser> presetChooser;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttach,thresholdAttach;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellyDeEsserAudioProcessorEditor)
};
