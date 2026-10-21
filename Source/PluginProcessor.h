#pragma once
#include <JuceHeader.h>
#include "dsp/DeEsserEngine.h"
class SkellyDeEsserAudioProcessor final : public juce::AudioProcessor {
public:
 SkellyDeEsserAudioProcessor();
 ~SkellyDeEsserAudioProcessor() override=default;
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "Skelly DeEsser";}
 bool acceptsMidi() const override{return false;}
 bool producesMidi() const override{return false;}
 bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds()const override{return 0.;}
 int getNumPrograms() override{return 1;}
 int getCurrentProgram() override{return 0;}
 void setCurrentProgram(int) override{}
 const juce::String getProgramName(int) override{return {};}
 void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
 juce::AudioProcessorValueTreeState apvts;
 DeEsserEngine engine;
 void restoreDefaults();
 void switchSlot(int nextSlot);
 void copyAToB();
 int getSlot() const noexcept { return activeSlot; }
private:
 int activeSlot=0;
 juce::ValueTree slotA,slotB;
 void saveActiveSlot();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellyDeEsserAudioProcessor)
};
