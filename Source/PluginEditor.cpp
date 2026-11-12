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
 stageArt=load(BinaryData::stage_bg_png,BinaryData::stage_bg_pngSize);
 leftArt=load(BinaryData::left_panel_bg_png,BinaryData::left_panel_bg_pngSize);
 faderArt=load(BinaryData::fader_housing_png,BinaryData::fader_housing_pngSize);
 capArt=load(BinaryData::fader_cap_png,BinaryData::fader_cap_pngSize);
 knobArt=load(BinaryData::knob_skull_png,BinaryData::knob_skull_pngSize);
 // Assemble the high-resolution static artwork just once. The 30-Hz animation
 // only needs to draw the cached image and the live controls/meters.
 staticBackdrop = juce::Image(juce::Image::ARGB, Theme::width, Theme::height, true);
 {
  juce::Graphics art(staticBackdrop);
  art.fillAll(Theme::black);
  // Keep the detailed artwork, but move it behind clean, dark control surfaces.
  // The screenshot from V6 was difficult to read because the stage texture,
  // framed sidebar and a second emblem were competing with the real controls.
  {
   juce::Graphics::ScopedSaveState save(art);
   art.setOpacity(0.46f);
   draw(art,stageArt,352,213,1137,650);
  }
  art.setColour(Theme::black.withAlpha(0.67f));
  art.fillRoundedRectangle(369.f,224.f,1096.f,615.f,12.f);
  {
   juce::Graphics::ScopedSaveState save(art);
   art.setOpacity(0.49f);
   draw(art,leftArt,51,198,254,659);
  }
  art.setColour(Theme::black.withAlpha(0.69f));
  art.fillRoundedRectangle(66.f,211.f,218.f,634.f,7.f);
  // One emblem only. The frame PNG already contains the center skull.
  // An additional large skull was causing double edges and visual clutter.
  {
   juce::Graphics::ScopedSaveState save(art);
   art.setOpacity(.65f);  // Let readable controls lead; original metal remains visible.
   draw(art,artFrame,6,6,1524,1012);
  }
 }
 look.setArt(knobArt,capArt);
 setLookAndFeel(&look);
 for(auto* c:{(juce::Component*)&frequency,(juce::Component*)&threshold,(juce::Component*)&frequencyValue,(juce::Component*)&thresholdValue,
  (juce::Component*)&split,(juce::Component*)&wide,(juce::Component*)&phraseButton,(juce::Component*)&audio,(juce::Component*)&sidechain,
  (juce::Component*)&hpf,(juce::Component*)&filter,(juce::Component*)&prev,(juce::Component*)&next,
  (juce::Component*)&save,(juce::Component*)&settings,(juce::Component*)&preset,
  (juce::Component*)&detectHeader,(juce::Component*)&frequencyHeader,(juce::Component*)&sideHeader,(juce::Component*)&monitorHeader,
  (juce::Component*)&waveform}) addAndMakeVisible(*c);
 frequency.setRange(1000,20000,1);frequency.setSkewFactorFromMidPoint(5860);
 threshold.setRange(-60,0,1);
 frequencyValue.setWheelStep(100.0);
 thresholdValue.setWheelStep(1.0);
 frequencyAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"frequency",frequency);
 thresholdAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"threshold",threshold);
 // Headless first-frame capture exposed empty readout boxes before the first
 // 24Hz timer tick. Keep editable values in sync on the JUCE message thread.
 frequency.onValueChange=[this]{
  if(!frequencyValue.isBeingEdited())
   frequencyValue.setText(juce::String((int)std::lround(frequency.getValue()))+" Hz",juce::dontSendNotification);
 };
 threshold.onValueChange=[this]{
  if(!thresholdValue.isBeingEdited())
   thresholdValue.setText(juce::String((int)std::lround(threshold.getValue()))+" dB",juce::dontSendNotification);
 };
 frequency.onValueChange();
 threshold.onValueChange();
 frequencyValue.onEditorHide=[this]{
  auto t=frequencyValue.getText().trim().toLowerCase().removeCharacters(" ");
  if(t.endsWith("hz"))t=t.dropLastCharacters(2);
  const bool khz=t.endsWithChar('k');
  if(khz)t=t.dropLastCharacters(1);
  if(t.isNotEmpty() && t.containsOnly("0123456789.")) {
   const auto value=t.getDoubleValue()*(khz?1000.0:1.0);
   if(std::isfinite(value)&&value>=1000.0&&value<=20000.0){
    setFloat("frequency",(float)value);
    frequencyValue.setText(juce::String((int)std::lround(frequency.getValue()))+" Hz",juce::dontSendNotification);
    return;
   }
  }
  frequencyValue.setText(juce::String((int)std::lround(frequency.getValue()))+" Hz",juce::dontSendNotification);
 };
 thresholdValue.onEditorHide=[this]{
  auto t=thresholdValue.getText().trim().toLowerCase().removeCharacters(" ");
  if(t.endsWith("db"))t=t.dropLastCharacters(2);
  if(t.isNotEmpty() && t.containsOnly("-0123456789.")){
   const double value=t.getDoubleValue();
   if(std::isfinite(value)&&value>=-60.0&&value<=0.0){
    setFloat("threshold",(float)std::round(value));
    thresholdValue.setText(juce::String((int)std::lround(threshold.getValue()))+" dB",juce::dontSendNotification);
    return;
   }
  }
  thresholdValue.setText(juce::String((int)std::lround(threshold.getValue()))+" dB",juce::dontSendNotification);
 };
 frequencyValue.onWheelDelta=[this](double delta){
  const double next=juce::jlimit(1000.0,20000.0,
     (double)processor.apvts.getRawParameterValue("frequency")->load()+delta);
  setFloat("frequency",(float)next);
  frequencyValue.setText(juce::String((int)std::lround(next))+" Hz",juce::dontSendNotification);
 };
 thresholdValue.onWheelDelta=[this](double delta){
  const double next=juce::jlimit(-60.0,0.0,
    std::round((double)processor.apvts.getRawParameterValue("threshold")->load()+delta));
  setFloat("threshold",(float)next);
  thresholdValue.setText(juce::String((int)std::lround(next))+" dB",juce::dontSendNotification);
 };
 split.onClick=[this]{setFloat("phraseAware",0.f);setChoice("detectMode",0);};
 wide.onClick=[this]{setFloat("phraseAware",0.f);setChoice("detectMode",1);};
 phraseButton.onClick=[this]{setChoice("detectMode",0);setFloat("phraseAware",1.f);updateButtons();repaint();};
 phraseButton.setTooltip("Phrase-Aware: learn a singer's normal sibilance per phrase; reduce outliers.");
 filter.setTooltip("Choose the detector filter type. Current choice is marked in the menu.");
 hpf.setTooltip("Enable or disable the sidechain high-pass filter.");
 settings.setTooltip("Audio controls, phrase sensitivity, window size, and plugin information.");
 preset.setTooltip("Import, export or reset a preset. A/B slots switch with the arrow buttons.");
 prev.setTooltip("Switch to the other A/B preset slot.");
 next.setTooltip("Switch to the other A/B preset slot.");
 save.setTooltip("Export the current A/B preset.");
 frequency.setTooltip("Detector frequency (1–20 kHz): drag knob, scroll, or edit Hz below.");
 threshold.setTooltip("Threshold: drag the skull. Double-click restores -24 dB.");
 frequencyValue.setTooltip("Enter 5860, 5.86k, or 5860 Hz and press Enter.");
 thresholdValue.setTooltip("Enter a value from -60 to 0 dB.");
 split.setTooltip("Split: reduce only the sibilance band; preserve low-frequency body.");
 wide.setTooltip("Wide: reduce the entire vocal when sibilance exceeds threshold.");
 audio.setTooltip("Listen to processed audio.");
 sidechain.setTooltip("Listen to the detector signal to fine-tune frequency.");
 audio.onClick=[this]{setChoice("monitorMode",0);};sidechain.onClick=[this]{setChoice("monitorMode",1);};
 hpf.onClick=[this]{applyDetectorFilterMode(1-getDetectorFilterMode());};
 filter.onClick=[this]{
  juce::PopupMenu m;
  m.setLookAndFeel(&look);
  m.addSectionHeader("Detector filter");
  const int choice=getDetectorFilterMode();
  m.addItem(1,"High-pass",true,choice==0);
  m.addItem(2,"Band-pass",true,choice==1);
  auto safe=juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&filter)
                 .withMinimumWidth(190).withMaximumNumColumns(1),
                 [safe](int selected) {
    if(safe && selected>=1 && selected<=2)
     safe->applyDetectorFilterMode(selected-1);
  });
 };
 settings.onClick=[this]{showSettings();};
 preset.onClick=[this]{
  juce::PopupMenu m;
  m.setLookAndFeel(&look);
  m.addSectionHeader("Preset and A/B");
  m.addItem(1,"Import .skelly preset...");
  m.addItem(2,"Export current .skelly preset...");
  m.addSeparator();
  m.addItem(3,"Reset processing to defaults");
  auto safe=juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&preset)
                   .withMinimumWidth(245).withMaximumNumColumns(1),[safe](int r){
   if(!safe)return;
   if(r==1)safe->loadPresetFile();
   else if(r==2)safe->savePresetFile();
   else if(r==3){safe->processor.restoreDefaults();safe->updateButtons();safe->repaint();}
  });
 };
 prev.onClick=next.onClick=[this]{processor.switchSlot(1-processor.getSlot());updateButtons();};
 save.onClick=[this]{savePresetFile();};
 aspect.setFixedAspectRatio(1.5);
 aspect.setSizeLimits(540,360,1536,1024);
 setConstrainer(&aspect);
 setResizable(true,true);
 // 633 x 422 is 55% of the V6 1152 x 768 launch size (45% smaller).
 // Old sessions had no UI layout revision; migrate their saved dimensions once,
 // while preserving user resizing after the V7 layout has been installed.
 constexpr int compactWidth=594, compactHeight=396;
 const bool migrated=(bool)p.apvts.state.getProperty("compactLayoutV15",false);
 int savedWidth=(int)p.apvts.state.getProperty("editorWidth",compactWidth);
 int savedHeight=(int)p.apvts.state.getProperty("editorHeight",compactHeight);
 // Shrink the previous default only; preserve existing custom window sizes.
 if(!migrated && savedWidth==633 && savedHeight==422) {
  savedWidth=compactWidth; savedHeight=compactHeight;
 }
 savedWidth=juce::jlimit(540,1536,savedWidth);
 savedHeight=juce::roundToInt(savedWidth/1.5f);
 p.apvts.state.setProperty("compactLayoutV7",true,nullptr);
 p.apvts.state.setProperty("compactLayoutV15",true,nullptr);
 setSize(savedWidth,savedHeight);

 startTimerHz(24); // Lower idle repaint cost without reducing the audio processing rate.
}
SkellyDeEsserAudioProcessorEditor::~SkellyDeEsserAudioProcessorEditor(){
 // Popup desktop windows retain a LookAndFeel pointer; dismiss before
 // the editor-owned SkellyLookAndFeel is destroyed on host editor close.
 juce::PopupMenu::dismissAllActiveMenus();
 setLookAndFeel(nullptr);
}
void SkellyDeEsserAudioProcessorEditor::setFloat(const char* id,float v){
 if(!std::isfinite(v))return;
 if(auto*p=processor.apvts.getParameter(id)){
  auto*r=dynamic_cast<juce::RangedAudioParameter*>(p);
  if(r){
   const auto range=r->getNormalisableRange();
   v=range.snapToLegalValue(juce::jlimit(range.start,range.end,v));
   r->beginChangeGesture();r->setValueNotifyingHost(r->convertTo0to1(v));r->endChangeGesture();
  }}
}
void SkellyDeEsserAudioProcessorEditor::setChoice(const char*id,int n){
 setFloat(id,(float)n);updateButtons();repaint();
}
int SkellyDeEsserAudioProcessorEditor::getDetectorFilterMode() const {
 return (int)processor.apvts.getRawParameterValue("sidechainFilterType")->load()==3 ? 1 : 0;
}
void SkellyDeEsserAudioProcessorEditor::applyDetectorFilterMode(int choice) {
 // Keep all legacy host-automation choices, expose only two UI selections.
 if(choice==0){setFloat("sidechainHpf",1.f);setChoice("sidechainFilterType",2);}
 else {setFloat("sidechainHpf",0.f);setChoice("sidechainFilterType",3);}
 updateButtons();repaint();
}
void SkellyDeEsserAudioProcessorEditor::updateButtons(){
 auto get=[this](const char*id){return processor.apvts.getRawParameterValue(id)->load();};
 const bool phraseOn=get("phraseAware")>.5f;
 split.setToggleState(!phraseOn && get("detectMode")<.5f,juce::dontSendNotification);
 wide.setToggleState(!phraseOn && get("detectMode")>.5f,juce::dontSendNotification);
 phraseButton.setToggleState(phraseOn,juce::dontSendNotification);
 audio.setToggleState(get("monitorMode")<.5f,juce::dontSendNotification);
 sidechain.setToggleState(get("monitorMode")>.5f,juce::dontSendNotification);
 const bool bp=getDetectorFilterMode()==1;
 hpf.setToggleState(bp,juce::dontSendNotification);
 hpf.setButtonText(bp?"BP":"HP");
 hpf.setTooltip(bp?"Band-pass: click to switch to High-pass."
                  :"High-pass: click to switch to Band-pass.");
 filter.setTooltip("Choose High-pass or Band-pass.");
 const auto text=processor.apvts.state.getProperty("slot","A").toString()+": "+
                      processor.apvts.state.getProperty("presetName","My Default Preset").toString();
 if(preset.getButtonText()!=text)preset.setButtonText(text);
}
// Portable preset files wrap the same host-compatible state document that
// already stores both A/B slots, the selected slot, and editor properties.
void SkellyDeEsserAudioProcessorEditor::savePresetFile(){
 const auto defaultName=processor.apvts.state.getProperty("presetName","My Default Preset").toString();
 auto destination=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile(defaultName + ".skelly");
 presetChooser=std::make_unique<juce::FileChooser>("Export Skelly DeEsser preset",destination,"*.skelly");
 const auto safe=juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
 presetChooser->launchAsync(juce::FileBrowserComponent::saveMode |
                            juce::FileBrowserComponent::canSelectFiles |
                            juce::FileBrowserComponent::warnAboutOverwriting,
  [safe](const juce::FileChooser& chooser){
   if(!safe)return;
   auto file=chooser.getResult();
   if(file==juce::File{})return;
   if(!file.hasFileExtension(".skelly"))file=file.withFileExtension(".skelly");
   auto& p=safe->processor;
   p.apvts.state.setProperty("presetName",file.getFileNameWithoutExtension(),nullptr);
   juce::MemoryBlock data;
   p.getStateInformation(data);
   if(!file.replaceWithData(data.getData(),data.getSize()))
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Skelly DeEsser",
                                           "Could not export preset to the selected location.");
   safe->updateButtons();
  });
}
void SkellyDeEsserAudioProcessorEditor::loadPresetFile(){
 auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
 presetChooser=std::make_unique<juce::FileChooser>("Load Skelly DeEsser preset",folder,"*.skelly");
 const auto safe=juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
 presetChooser->launchAsync(juce::FileBrowserComponent::openMode |
                            juce::FileBrowserComponent::canSelectFiles,
  [safe](const juce::FileChooser& chooser){
   if(!safe)return;
   auto file=chooser.getResult();
   if(file==juce::File{})return;
   juce::MemoryBlock data;
   if(file.getSize()<1 || file.getSize()>4*1024*1024 || !file.loadFileAsData(data)){
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Skelly DeEsser",
                                           "Preset file is missing, unreadable or too large.");
    return;
   }
   if(!safe->processor.loadValidatedPresetData(data.getData(),(int)data.getSize())){
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Skelly DeEsser",
                                           "This is not a valid Skelly DeEsser preset.");
    return;
   }
   safe->updateButtons();
  });
}
void SkellyDeEsserAudioProcessorEditor::showSettings(){
 // Keep the top-level menu short enough for the 540x360 minimum window.
 // More detailed controls stay grouped in small, keyboard-navigable submenus.
 juce::PopupMenu m;
 juce::PopupMenu processing,phrase,presets,sizes;
 for(auto* menu:{&m,&processing,&phrase,&presets,&sizes})
  menu->setLookAndFeel(&look);
 struct MenuParam {const char* key;const char* name;const char* units;float step;int base;};
 constexpr MenuParam menuParams[] = {
  {"ratio","Ratio",":1",.5f,100},
  {"range","Maximum reduction"," dB",1.f,110},
  {"attack","Attack"," ms",.5f,120},
  {"release","Release"," ms",10.f,130},
  {"stereoLink","Stereo link","%",10.f,140},
  {"outputGain","Output gain"," dB",.5f,150},
  {"phraseSensitivity","Phrase sensitivity"," dB above normal",.5f,160}
 };
 for(const auto& p:menuParams){
  const auto value=processor.apvts.getRawParameterValue(p.key)->load();
  juce::PopupMenu adjustments;
  adjustments.setLookAndFeel(&look);
  adjustments.addItem(p.base+1,"Decrease  " + juce::String(p.step,1) + p.units);
  adjustments.addItem(p.base+2,"Increase  " + juce::String(p.step,1) + p.units);
  adjustments.addItem(p.base+3,"Restore default");
  const auto title=juce::String(p.name)+"   "+juce::String(value,1)+p.units;
  if(p.base==160)phrase.addSubMenu(title,adjustments);
  else processing.addSubMenu(title,adjustments);
 }
 m.addSectionHeader("Skelly DeEsser settings");
 m.addSubMenu("Processing controls",processing);
 m.addSubMenu("Phrase-Aware",phrase);
 sizes.addItem(201,"Compact",true,getWidth()==594);
 sizes.addItem(202,"Medium",true,getWidth()==760);
 sizes.addItem(203,"Large",true,getWidth()==920);
 sizes.addItem(204,"Original",true,getWidth()==1080);
 m.addSubMenu("Window size",sizes);
 presets.addItem(1,"Reset processing to defaults");
 presets.addItem(2,"Copy A to B");
 m.addSubMenu("Preset actions",presets);
 m.addSeparator();
 m.addItem(9,"About Skelly DeEsser V15");
 const auto safe=juce::Component::SafePointer<SkellyDeEsserAudioProcessorEditor>(this);
 m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settings)
                  .withMinimumWidth(240).withMaximumNumColumns(1),[safe](int r){
  if(!safe)return;
  auto& self=*safe;
  if(r==1){self.processor.restoreDefaults();self.updateButtons();self.repaint();return;}
  if(r==2){self.processor.copyAToB();self.updateButtons();return;}
  if(r>=201 && r<=204){
   constexpr int widths[]{594,760,920,1080};
   const int width=widths[r-201];
   self.setSize(width,juce::roundToInt(width/1.5f));
   return;
  }
  if(r==9){
   juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"Skelly DeEsser V15",
    "Gothic stereo de-esser. Split, Wide and Phrase-Aware, zero reported latency, and host automation.");
   return;
  }
  constexpr struct ParamEntry {const char* key;float step;int base;} entries[] = {
   {"ratio",.5f,100},{"range",1.f,110},{"attack",.5f,120},
   {"release",10.f,130},{"stereoLink",10.f,140},{"outputGain",.5f,150},
   {"phraseSensitivity",.5f,160}
  };
  for(const auto& p:entries) {
   if(r>=p.base+1 && r<=p.base+3){
    auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(self.processor.apvts.getParameter(p.key));
    if(parameter==nullptr)return;
    if(r==p.base+3){
     parameter->beginChangeGesture();
     parameter->setValueNotifyingHost(parameter->getDefaultValue());
     parameter->endChangeGesture();
    }else{
     const float delta=(r==p.base+1 ? -p.step : p.step);
     self.setFloat(p.key,self.processor.apvts.getRawParameterValue(p.key)->load()+delta);
    }
    return;
   }
  }
 });
}
void SkellyDeEsserAudioProcessorEditor::paint(juce::Graphics&g){
 const float sx=getWidth()/1536.f,sy=getHeight()/1024.f;
 g.addTransform(juce::AffineTransform::scale(sx,sy));
 // All layered gothic artwork is composited in staticBackdrop at construction.
 if(staticBackdrop.isValid())
  draw(g,staticBackdrop,0,0,1536,1024);
 else g.fillAll(Theme::black);

 // All labels, meter graphics and user controls remain dynamic and sharp.
 // Quiet recessed rails reinforce the layout without covering the waveform.
 g.setColour(juce::Colour::fromRGB(5,18,9).withAlpha(.69f));
 g.fillRoundedRectangle(891.f,779.f,556.f,58.f,9.f);
 g.setColour(Theme::mid.withAlpha(.55f));
 g.drawRoundedRectangle(891.f,779.f,556.f,58.f,9.f,1.25f);
 g.setColour(Theme::neon.withAlpha(.12f));
 g.setFont(Theme::font(60,true,false));
 g.drawText("SKELLY",78,64,292,76,juce::Justification::centredLeft);
 g.setColour(Theme::neon);
 g.setFont(Theme::font(58,true,false));
 g.drawText("SKELLY",82,60,292,76,juce::Justification::centredLeft);
 title(g,"DE-ESSER",{104,136,220,28},21);
 title(g,"THRESHOLD",{676,245,191,42},28);
 // This rail is sidechain level; the separate LED strip is gain reduction.
 g.setColour(Theme::label.withAlpha(.8f));
 g.setFont(Theme::font(23,true));
 g.drawFittedText("SC LEVEL",{900,289,148,27},juce::Justification::centred,1);
 Theme::line(g,576,267,678,267,.65f);
 Theme::line(g,866,267,965,267,.65f);

 // The threshold cap is a real slider child. Only the live sidechain meter and
 // housing are painted here; the cap is then rendered above by LookAndFeel.
 draw(g,faderArt,729,296,80,480);
 const float sideDb=juce::jlimit(-60.f,0.f,processor.engine.sidechainLevelDb.load());
 const float fill=juce::jlimit(0.f,1.f,(sideDb+60.f)/60.f);
 constexpr float channelX=753.f,channelW=30.f,top=314.f,bottom=755.f;
 const float fillHeight=(bottom-top)*fill;
 g.setColour(Theme::black.withAlpha(.52f));
 g.fillRoundedRectangle(channelX,top,channelW,bottom-top,3.f);
 if(fillHeight>0.3f){
  g.setColour(Theme::neon.withAlpha(.21f));
  g.fillRoundedRectangle(channelX-3.f,bottom-fillHeight,channelW+6.f,fillHeight,4.f);
  juce::ColourGradient grad(juce::Colour::fromRGB(182,255,42),768.f,bottom-fillHeight,
                            Theme::neon,768.f,bottom,false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(channelX,bottom-fillHeight,channelW,fillHeight,2.f);
 }
 for(int i=0;i<=30;++i){
  const float y=310.f+i*(445.f/30.f);
  Theme::line(g,i%5?812.f:808.f,y,i%5?819.f:827.f,y,i%5?.45f:.90f);
  if(i%5==0)title(g,juce::String(-2*i),{830,(int)y-14,63,27},22);
 }
 // The 1/20 kHz labels overlapped the readout in the compact layout.
 // The knob tooltip specifies its range, and the actual value stays visible.

 const auto reduction=juce::jmax(0.f,processor.engine.gainReductionDb.load());
 // Separate 24 dB gain-reduction LED meter; the tall central meter is SC level.
 // The separate meter prevents users confusing detector level and actual GR.
 constexpr float grX=1180.f,grY=773.f,grWidth=260.f;
 for(int seg=0;seg<24;++seg){
  const auto rect=juce::Rectangle<float>(grX+seg*(grWidth/24.f),grY,8.6f,14.f);
  const bool active=reduction >= (float)(seg+1);
  g.setColour(active ? (seg<12 ? Theme::neon : juce::Colour::fromRGB(191,255,54)) : Theme::dim.withAlpha(.52f));
  g.fillRoundedRectangle(rect,2.f);
 }
 // A compact, actual detector-state readout (never a simulated waveform
 // or an invented "AI" confidence value). Only visible in Phrase mode.
 if (processor.apvts.getRawParameterValue("phraseAware")->load(std::memory_order_relaxed) > .5f) {
  const auto events=processor.engine.phraseLearnedEvents.load(std::memory_order_relaxed);
  const auto baseline=processor.engine.phraseBaselineDb.load(std::memory_order_relaxed);
  juce::String msg="PHRASE  |  LEARNING";
  if (events>0)
   msg="PHRASE  |  NORMAL " + juce::String(baseline,1) + " dBFS  |  "
       + juce::String(events) + " EVENTS";
  g.setColour(Theme::bright.withAlpha(.78f));
  g.setFont(Theme::font(25,true));
  g.drawFittedText(msg,{932,729,506,38},juce::Justification::centredRight,1);
 }
 // Readouts stay on separate unobstructed lines near the bottom of the stage.
 g.setColour(Theme::label);
 g.setFont(Theme::font(24,true));
 g.drawFittedText("SC  " + juce::String(sideDb,1) + " dBFS",
                  {917,792,220,43},juce::Justification::centredRight,1);
 Theme::line(g,1158,799,1158,830,.35f);
 g.setColour(reduction>.05f?Theme::bright:Theme::label);
 g.drawFittedText("GR  " + juce::String(reduction,1) + " dB",
                  {1191,792,241,43},juce::Justification::centredLeft,1);
}

void SkellyDeEsserAudioProcessorEditor::resized(){
 const auto sx=getWidth()/1536.f,sy=getHeight()/1024.f;
 auto place=[sx,sy](juce::Component&c,int x,int y,int w,int h){c.setBounds(juce::roundToInt(x*sx),juce::roundToInt(y*sy),juce::roundToInt(w*sx),juce::roundToInt(h*sy));};
 place(detectHeader,76,211,209,41);
 place(split,91,259,178,32);place(wide,91,294,178,32);place(phraseButton,91,329,178,32);
 // Keep both monitor buttons within the inner gothic panel (ends near y=850).
 // This column is grouped more tightly without shrinking the knob labels.
 place(frequencyHeader,76,365,209,37);place(frequency,104,406,150,125);
 place(frequencyValue,100,536,158,37);place(sideHeader,76,583,209,35);
 place(hpf,89,629,122,48);place(filter,220,629,52,48);
 place(monitorHeader,76,694,209,35);place(audio,89,738,183,44);
 place(sidechain,89,788,183,44);
 place(waveform,397,316,1064,418);waveform.toBack();
 place(threshold,710,304,116,464);place(thresholdValue,684,783,168,49);
 // Compact preset header with the right-side metal border unobstructed.
 place(prev,1026,128,50,54);place(preset,1080,128,276,54);
 place(next,1360,128,50,54);place(save,1414,128,38,54);place(settings,1456,128,36,54);
 const auto textPx=juce::jmax(10.f,24.f*sy);
 frequencyValue.setFont(Theme::font(textPx,true));
 thresholdValue.setFont(Theme::font(juce::jmax(11.f,28.f*sy),true));
 processor.apvts.state.setProperty("editorWidth",getWidth(),nullptr);
 processor.apvts.state.setProperty("editorHeight",getHeight(),nullptr);
}
void SkellyDeEsserAudioProcessorEditor::timerCallback(){
 updateButtons();
 const auto frequencyText=juce::String((int)std::lround(frequency.getValue()))+" Hz";
 const auto thresholdText=juce::String((int)std::lround(threshold.getValue()))+" dB";
 if(!frequencyValue.isBeingEdited() && frequencyValue.getText()!=frequencyText)
  frequencyValue.setText(frequencyText,juce::dontSendNotification);
 if(!thresholdValue.isBeingEdited() && thresholdValue.getText()!=thresholdText)
  thresholdValue.setText(thresholdText,juce::dontSendNotification);
 const auto serial=processor.engine.waveform.serial();
 const float side=processor.engine.sidechainLevelDb.load(std::memory_order_relaxed);
 const float gr=processor.engine.gainReductionDb.load(std::memory_order_relaxed);
 const float phraseDb=processor.engine.phraseBaselineDb.load(std::memory_order_relaxed);
 const int learned=processor.engine.phraseLearnedEvents.load(std::memory_order_relaxed);
 const bool phraseOn=processor.apvts.getRawParameterValue("phraseAware")->load(std::memory_order_relaxed)>.5f;
 const bool waveChanged=serial!=lastWaveSerial;
 const bool meterChanged=std::abs(side-lastSideDb)>.05f || std::abs(gr-lastGrDb)>.05f;
 const bool phraseChanged=phraseOn!=lastPhraseOn || learned!=lastLearnedEvents ||
                          (phraseOn && std::abs(phraseDb-lastPhraseDb)>.05f);
 if(waveChanged) { lastWaveSerial=serial; waveform.advance(); }
 if(waveChanged||meterChanged||phraseChanged) {
  lastSideDb=side;lastGrDb=gr;lastPhraseDb=phraseDb;
  lastLearnedEvents=learned;lastPhraseOn=phraseOn;
  repaint();
 }
}
