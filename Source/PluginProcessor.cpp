#include "PluginProcessor.h"
#include "PluginEditor.h"
using APF=juce::AudioParameterFloat;
using APC=juce::AudioParameterChoice;
using APB=juce::AudioParameterBool;
static auto id(const char* x){return juce::ParameterID(x,1);}
juce::AudioProcessorValueTreeState::ParameterLayout SkellyDeEsserAudioProcessor::makeParameters(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 l.add(std::make_unique<APF>(id("threshold"),"Threshold",juce::NormalisableRange<float>(-60.f,0.f,.1f),-24.f));
 auto freq=juce::NormalisableRange<float>(1000.f,20000.f,1.f);freq.setSkewForCentre(5860.f);
 l.add(std::make_unique<APF>(id("frequency"),"Frequency",freq,5860.f));
 l.add(std::make_unique<APC>(id("detectMode"),"Detection",juce::StringArray{"Split","Wide"},0));
 l.add(std::make_unique<APB>(id("sidechainHpf"),"Sidechain HPF",true));
 l.add(std::make_unique<APC>(id("sidechainFilterType"),"Sidechain Filter",juce::StringArray{"Off","HPF 6","HPF 12","Band-Pass","Listen"},2));
 l.add(std::make_unique<APC>(id("monitorMode"),"Monitor",juce::StringArray{"Audio","Sidechain"},0));
 l.add(std::make_unique<APF>(id("ratio"),"Ratio",juce::NormalisableRange<float>(1.f,20.f,.1f),4.f));
 l.add(std::make_unique<APF>(id("range"),"Max Reduction",juce::NormalisableRange<float>(0.f,24.f,.1f),12.f));
 l.add(std::make_unique<APF>(id("attack"),"Attack",juce::NormalisableRange<float>(.1f,30.f,.1f),2.f));
 l.add(std::make_unique<APF>(id("release"),"Release",juce::NormalisableRange<float>(10.f,500.f,1.f),85.f));
 l.add(std::make_unique<APF>(id("stereoLink"),"Stereo Link",juce::NormalisableRange<float>(0.f,100.f,1.f),100.f));
 l.add(std::make_unique<APF>(id("outputGain"),"Output",juce::NormalisableRange<float>(-12.f,12.f,.1f),0.f));
 return l;
}
SkellyDeEsserAudioProcessor::SkellyDeEsserAudioProcessor():
 AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 apvts(*this,nullptr,"SkellyState",makeParameters()){
 apvts.state.setProperty("presetName","My Default Preset",nullptr);
 apvts.state.setProperty("slot","A",nullptr);
 apvts.state.setProperty("editorWidth",1152,nullptr);
 apvts.state.setProperty("editorHeight",768,nullptr);
 slotA=apvts.copyState();slotB=slotA.createCopy();
}
void SkellyDeEsserAudioProcessor::prepareToPlay(double sampleRate,int){engine.prepare(sampleRate);setLatencySamples(0);}
bool SkellyDeEsserAudioProcessor::isBusesLayoutSupported(const BusesLayout& l)const{
 auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
 return in==out&&(out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo());
}
void SkellyDeEsserAudioProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals;
 for(int c=getTotalNumInputChannels();c<audio.getNumChannels();++c)audio.clear(c,0,audio.getNumSamples());
 auto get=[this](const char* name){return apvts.getRawParameterValue(name)->load();};
 DeEsserEngine::Settings s;
 s.threshold=get("threshold");s.frequency=get("frequency");s.ratio=get("ratio");s.range=get("range");
 s.attack=get("attack");s.release=get("release");s.link=get("stereoLink");s.output=get("outputGain");
 s.mode=(int)get("detectMode");s.filterType=(int)get("sidechainFilterType");s.monitor=(int)get("monitorMode");s.hpf=get("sidechainHpf")>.5f;
 engine.process(audio,s);
}
juce::AudioProcessorEditor* SkellyDeEsserAudioProcessor::createEditor(){return new SkellyDeEsserAudioProcessorEditor(*this);}
void SkellyDeEsserAudioProcessor::saveActiveSlot(){
 if(activeSlot==0)slotA=apvts.copyState(); else slotB=apvts.copyState();
}
void SkellyDeEsserAudioProcessor::switchSlot(int nextSlot){
 nextSlot=juce::jlimit(0,1,nextSlot);
 if(nextSlot==activeSlot)return;
 saveActiveSlot();
 activeSlot=nextSlot;
 auto snapshot=activeSlot==0?slotA:slotB;
 if(snapshot.isValid())apvts.replaceState(snapshot.createCopy());
 apvts.state.setProperty("slot",activeSlot==0?"A":"B",nullptr);
}
void SkellyDeEsserAudioProcessor::copyAToB(){
 if(activeSlot==0)saveActiveSlot();
 slotB=slotA.createCopy();
 if(activeSlot==1){apvts.replaceState(slotB.createCopy());apvts.state.setProperty("slot","B",nullptr);}
}
void SkellyDeEsserAudioProcessor::getStateInformation(juce::MemoryBlock& dst){
 saveActiveSlot();
 juce::ValueTree root("SkellyDocument");
 root.addChild(apvts.copyState(),-1,nullptr);
 juce::ValueTree a("SlotA");a.copyPropertiesAndChildrenFrom(slotA,nullptr);root.addChild(a,-1,nullptr);
 juce::ValueTree b("SlotB");b.copyPropertiesAndChildrenFrom(slotB,nullptr);root.addChild(b,-1,nullptr);
 root.setProperty("activeSlot",activeSlot,nullptr);
 if(auto xml=root.createXml())copyXmlToBinary(*xml,dst);
}
void SkellyDeEsserAudioProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size)){
  const auto tree=juce::ValueTree::fromXml(*xml);
  if(tree.hasType(juce::Identifier("SkellyDocument"))){
   auto current=tree.getChild(0);
   if(current.isValid()&&current.hasType(apvts.state.getType()))apvts.replaceState(current.createCopy());
   slotA=juce::ValueTree(apvts.state.getType());
   if(auto a=tree.getChildWithName("SlotA");a.isValid())slotA.copyPropertiesAndChildrenFrom(a,nullptr);
   else slotA=apvts.copyState();
   slotB=juce::ValueTree(apvts.state.getType());
   if(auto b=tree.getChildWithName("SlotB");b.isValid())slotB.copyPropertiesAndChildrenFrom(b,nullptr);
   else slotB=apvts.copyState();
   activeSlot=juce::jlimit(0,1,(int)tree.getProperty("activeSlot",0));
   if(!slotA.isValid())slotA=apvts.copyState();
   if(!slotB.isValid())slotB=apvts.copyState();
  }else if(tree.isValid()&&tree.hasType(apvts.state.getType())){
   apvts.replaceState(tree);slotA=apvts.copyState();slotB=slotA.createCopy();
   activeSlot=apvts.state.getProperty("slot","A").toString()=="B"?1:0;
  }
 }
}
void SkellyDeEsserAudioProcessor::restoreDefaults(){
 for(auto* p:getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(p)){
  ranged->beginChangeGesture();ranged->setValueNotifyingHost(ranged->getDefaultValue());ranged->endChangeGesture();
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SkellyDeEsserAudioProcessor();}
