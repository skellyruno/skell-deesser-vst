#include "PluginEditor.h"
#include <BinaryData.h>
namespace {
juce::Image load(const void*ptr,int n){return juce::ImageFileFormat::loadFrom(ptr,(size_t)n);}
void draw(juce::Graphics&g,const juce::Image&i,float x,float y,float w,float h){if(i.isValid())g.drawImage(i,(int)x,(int)y,(int)w,(int)h,0,0,i.getWidth(),i.getHeight());}
void title(juce::Graphics&g,const juce::String&txt,juce::Rectangle<int>r,int size=20){g.setColour(Theme::label);
 g.setFont(Theme::font((float)size,true));g.drawFittedText(txt,r,juce::Justification::centred,1);}
}
SkellyDeEsserAudioProcessorEditor::SkellyDeEsserAudioProcessorEditor(SkellyDeEsserAudioProcessor& p):
 AudioProcessorEditor(&p),processor(p),waveform(p.engine.waveform){
 artFrame=load(BinaryData::frame_png,BinaryData::frame_pngSize);
 artSkull=load(BinaryData::skull_emblem_png,BinaryData::skull_emblem_pngSize);
 stageArt=load(BinaryData::stage_bg_png,BinaryData::stage_bg_pngSize);
 leftArt=load(BinaryData::left_panel_bg_png,BinaryData::left_panel_bg_pngSize);
 faderArt=load(BinaryData::fader_housing_png,BinaryData::fader_housing_pngSize);
 capArt=load(BinaryData::fader_cap_png,BinaryData::fader_cap_pngSize);
 knobArt=load(BinaryData::knob_skull_png,BinaryData::knob_skull_pngSize);
 look.setArt(knobArt,capArt);
 setLookAndFeel(&look);
 for(auto* c:{(juce::Component*)&frequency,(juce::Component*)&threshold,(juce::Component*)&frequencyValue,(juce::Component*)&thresholdValue,
  (juce::Component*)&split,(juce::Component*)&wide,(juce::Component*)&audio,(juce::Component*)&sidechain,
  (juce::Component*)&hpf,(juce::Component*)&filter,(juce::Component*)&prev,(juce::Component*)&next,
  (juce::Component*)&save,(juce::Component*)&settings,(juce::Component*)&preset,
  (juce::Component*)&detectHeader,(juce::Component*)&frequencyHeader,(juce::Component*)&sideHeader,(juce::Component*)&monitorHeader,
  (juce::Component*)&waveform}) addAndMakeVisible(*c);
 frequency.setRange(1000,20000,1);frequency.setSkewFactorFromMidPoint(5860);
 threshold.setRange(-60,0,.1);
 frequencyAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"frequency",frequency);
 thresholdAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"threshold",threshold);
 frequencyValue.onTextChange=[this]{auto v=frequencyValue.getText().getFloatValue();if(v>0)setFloat("frequency",v);};
 thresholdValue.onTextChange=[this]{setFloat("threshold",thresholdValue.getText().getFloatValue());};
 split.onClick=[this]{setChoice("detectMode",0);};wide.onClick=[this]{setChoice("detectMode",1);};
 audio.onClick=[this]{setChoice("monitorMode",0);};sidechain.onClick=[this]{setChoice("monitorMode",1);};
 hpf.onClick=[this]{auto*v=processor.apvts.getRawParameterValue("sidechainHpf");setFloat("sidechainHpf",v->load()>.5f?0.f:1.f);};
 filter.onClick=[this]{juce::PopupMenu m;juce::StringArray names{"Off","HPF 6 dB","HPF 12 dB","Band-Pass","Listen Filter"};
  for(int i=0;i<names.size();++i)m.addItem(i+1,names[i],true,i==(int)processor.apvts.getRawParameterValue("sidechainFilterType")->load());
  auto safe = juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
   m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&filter),[safe](int r){if(safe&&r>0)safe->setChoice("sidechainFilterType",r-1);});};
 settings.onClick=[this]{showSettings();};
 preset.onClick=[this]{juce::PopupMenu m;m.addItem(1,"My Default Preset");m.addItem(2,"Init");
  auto safe = juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
   m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&preset),[safe](int r){if(safe&&r==2)safe->processor.restoreDefaults();});};
 prev.onClick=next.onClick=[this]{processor.switchSlot(1-processor.getSlot());updateButtons();};
 save.onClick=[this]{juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"Skelly DeEsser","Preset and A/B state are saved with your host project.");};
 aspect.setFixedAspectRatio(1.5);setConstrainer(&aspect);setResizable(true,true);
 setSize((int)p.apvts.state.getProperty("editorWidth",1152),(int)p.apvts.state.getProperty("editorHeight",768));

 startTimerHz(30);
}
SkellyDeEsserAudioProcessorEditor::~SkellyDeEsserAudioProcessorEditor(){setLookAndFeel(nullptr);}
void SkellyDeEsserAudioProcessorEditor::setFloat(const char* id,float v){
 if(auto*p=processor.apvts.getParameter(id)){
  auto*r=dynamic_cast<juce::RangedAudioParameter*>(p);
  if(r){r->beginChangeGesture();r->setValueNotifyingHost(r->convertTo0to1(v));r->endChangeGesture();}}
}
void SkellyDeEsserAudioProcessorEditor::setChoice(const char*id,int n){setFloat(id,(float)n);updateButtons();}
void SkellyDeEsserAudioProcessorEditor::updateButtons(){
 auto get=[this](const char*id){return processor.apvts.getRawParameterValue(id)->load();};
 split.setToggleState(get("detectMode")<.5f,juce::dontSendNotification);
 wide.setToggleState(get("detectMode")>.5f,juce::dontSendNotification);
 audio.setToggleState(get("monitorMode")<.5f,juce::dontSendNotification);
 sidechain.setToggleState(get("monitorMode")>.5f,juce::dontSendNotification);
 hpf.setToggleState(get("sidechainHpf")>.5f,juce::dontSendNotification);
 preset.setButtonText(processor.apvts.state.getProperty("slot","A").toString()+": "+
                      processor.apvts.state.getProperty("presetName","My Default Preset").toString());
}
void SkellyDeEsserAudioProcessorEditor::showSettings(){
 juce::PopupMenu m;m.addItem(1,"Init");m.addItem(2,"Copy A to B");m.addSeparator();
 m.addSectionHeader("DSP controls (host automation available)");
 m.addItem(3,"Ratio +0.5");m.addItem(4,"Max reduction +1 dB");
 m.addItem(5,"Attack +1 ms");m.addItem(6,"Release +10 ms");
 m.addItem(7,"Stereo link 100%");m.addItem(8,"Output gain 0 dB");
 m.addSeparator();m.addItem(9,"About Skelly DeEsser 1.0");
 auto safe = juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settings),[safe](int r){
  if(!safe)return;
  auto&self=*safe;
  auto add=[&](const char*k,float step){self.setFloat(k,self.processor.apvts.getRawParameterValue(k)->load()+step);};
  switch(r){case 1:self.processor.restoreDefaults();break;
  case 2:self.processor.copyAToB();break;
  case 3:add("ratio",.5f);break;case 4:add("range",1);break;case 5:add("attack",1);break;
  case 6:add("release",10);break;case 7:self.setFloat("stereoLink",100);break;
  case 8:self.setFloat("outputGain",0);break;
  case 9:juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"Skelly DeEsser","Original stereo de-esser — JUCE / Skelly Audio");break;
  default:break;}
 });
}
void SkellyDeEsserAudioProcessorEditor::paint(juce::Graphics&g){
 g.fillAll(Theme::black);
 const float sx=getWidth()/1536.f,sy=getHeight()/1024.f;
 g.addTransform(juce::AffineTransform::scale(sx,sy));
 draw(g,stageArt,352,213,1137,650);draw(g,leftArt,51,198,254,659);
 g.setColour(Theme::dim.withAlpha(.65f));g.drawRect(350,216,1138,643,1);
 for(int y=312;y<845;y+=93)Theme::line(g,353,(float)y,1486,(float)y,.09f);
 // Keep artwork prominent while preserving contrast with functional overlays.
 draw(g,artFrame,0,0,1536,1024);
 draw(g,artSkull,624,0,288,242);
 g.setColour(Theme::neon);g.setFont(Theme::font(64,true));g.drawText("SKELLY",80,67,300,77,juce::Justification::centredLeft);
 title(g,"DeEsser",{120,135,195,35},25);
 title(g,"THRESHOLD",{677,245,193,42},26);
 Theme::line(g,576,267,678,267,.8f);Theme::line(g,867,267,964,267,.8f);
 draw(g,faderArt,733,295,75,482);
 const float sideDb=processor.engine.sidechainLevelDb.load();
 const float fill=juce::jlimit(0.f,1.f,(sideDb+60.f)/60.f);
 const float yBottom=758.f,fillHeight=445.f*fill;
 juce::ColourGradient grad(juce::Colour::fromRGB(182,255,42),767,yBottom-fillHeight,Theme::neon,767,yBottom,false);
 g.setGradientFill(grad);g.fillRoundedRectangle(748.f,yBottom-fillHeight,39.f,fillHeight,3.f);
 for(int i=0;i<=30;++i){float y=310.f+i*(445.f/30.f);
  Theme::line(g,i%5?811.f:808.f,y,i%5?820.f:828.f,y,i%5?.52f:1.f);
  if(i%5==0)title(g,juce::String(-2*i),{835,(int)y-14,65,27},20);
 }
 title(g,"1k",{75,529,43,24},15);title(g,"20k",{243,529,47,24},15);
 g.setColour(Theme::bright);g.setFont(Theme::font(18));g.drawText("GAIN REDUCTION  "+juce::String(processor.engine.gainReductionDb.load(),1)+" dB",1084,798,355,35,juce::Justification::centredRight);
}
void SkellyDeEsserAudioProcessorEditor::resized(){
 const auto sx=getWidth()/1536.f,sy=getHeight()/1024.f;
 auto place=[sx,sy](juce::Component&c,int x,int y,int w,int h){c.setBounds(juce::roundToInt(x*sx),juce::roundToInt(y*sy),juce::roundToInt(w*sx),juce::roundToInt(h*sy));};
 place(detectHeader,76,214,209,41);place(split,91,266,178,43);place(wide,91,313,178,43);
 place(frequencyHeader,76,368,209,42);place(frequency,104,424,150,130);
 place(frequencyValue,100,558,158,39);place(sideHeader,76,613,209,39);
 place(hpf,89,667,89,50);place(filter,182,667,90,50);
 place(monitorHeader,76,734,209,39);place(audio,76,791,101,54);place(sidechain,183,791,100,54);
 place(waveform,397,316,1064,418);waveform.toBack();
 place(threshold,710,304,116,464);place(thresholdValue,684,783,168,49);
 place(prev,1041,128,47,54);place(preset,1094,128,273,54);
 place(next,1336,130,34,50);place(save,1380,128,53,54);place(settings,1444,128,54,54);
 const auto textPx=juce::jmax(12.f,20.f*sy);
 frequencyValue.setFont(Theme::font(textPx,true));
 thresholdValue.setFont(Theme::font(juce::jmax(14.f,25.f*sy),true));
 processor.apvts.state.setProperty("editorWidth",getWidth(),nullptr);
 processor.apvts.state.setProperty("editorHeight",getHeight(),nullptr);
}
void SkellyDeEsserAudioProcessorEditor::timerCallback(){
 updateButtons();if (!frequencyValue.isBeingEdited())frequencyValue.setText(juce::String((int)frequency.getValue())+" Hz",juce::dontSendNotification);
 if (!thresholdValue.isBeingEdited())thresholdValue.setText(juce::String(threshold.getValue(),1)+" dB",juce::dontSendNotification);
 waveform.advance();repaint();
}
