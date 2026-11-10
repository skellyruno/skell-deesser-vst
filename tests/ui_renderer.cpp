// Development-only visual regression harness.
// It instantiates the production SkellyDeEsserAudioProcessor and its real
// SkellyDeEsserAudioProcessorEditor. No fake controls or mock UI.
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <SkellCore/HeadlessUIRenderer.h>
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
juce::Button* findButton(juce::Component& editor, const juce::String& label) {
  for(int i=0;i<editor.getNumChildComponents();++i)
    if(auto* b=dynamic_cast<juce::Button*>(editor.getChildComponent(i)))
      if(b->getButtonText()==label) return b;
  return nullptr;
}
template <typename T> T* findControl(juce::Component& editor) {
  for(int i=0;i<editor.getNumChildComponents();++i)
    if(auto* control=dynamic_cast<T*>(editor.getChildComponent(i)))return control;
  return nullptr;
}
float parameter(SkellyDeEsserAudioProcessor& processor,const char* id) {
  if(auto* p=processor.apvts.getRawParameterValue(id))
    return p->load(std::memory_order_relaxed);
  return -12345.f;
}
bool click(juce::Component& editor,const char* name) {
  // triggerClick() is async: a headless snapshot is taken before the
  // queued event executes. Call the actual editor-installed button handler
  // synchronously so assertions and scene configuration are deterministic.
  if(auto* b=findButton(editor,name)){
    if(!b->onClick)return false;
    b->onClick();
    return true;
  }
  return false;
}
void setParam(SkellyDeEsserAudioProcessor& processor,const char* id,float physical) {
  if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(processor.apvts.getParameter(id))) {
    const auto range=p->getNormalisableRange();
    p->beginChangeGesture();
    p->setValueNotifyingHost(p->convertTo0to1(
      range.snapToLegalValue(juce::jlimit(range.start,range.end,physical))));
    p->endChangeGesture();
  }
}
bool closeTo(float a,float b) {return std::abs(a-b)<.15f;}
int exerciseActualControls(SkellyDeEsserAudioProcessor& processor,
                           SkellyDeEsserAudioProcessorEditor& editor) {
  int failures=0;
  auto expect=[&](bool ok,const char* action) {
    if(!ok){std::fprintf(stderr,"UI interaction failed: %s\n",action);++failures;}
  };
  expect(click(editor,"Phrase") && parameter(processor,"phraseAware")>.5f,
         "Phrase button must enable phrase DSP");
  expect(click(editor,"Wide") && parameter(processor,"phraseAware")<.5f
         && closeTo(parameter(processor,"detectMode"),1.f),
         "Wide button must disable phrase DSP and choose wide");
  expect(click(editor,"Split") && closeTo(parameter(processor,"detectMode"),0.f),
         "Split button must choose split");
  expect(click(editor,"S Chain") && closeTo(parameter(processor,"monitorMode"),1.f),
         "Sidechain monitoring button must select detector");
  expect(click(editor,"Audio") && closeTo(parameter(processor,"monitorMode"),0.f),
         "Audio monitoring button must restore processed output");
  int readouts=0;
  for(int i=0;i<editor.getNumChildComponents();++i)
    if(auto* label=dynamic_cast<ReadoutBox*>(editor.getChildComponent(i)))
      if(label->getText().isNotEmpty())++readouts;
  expect(readouts==2,"Frequency and threshold readouts must be visible on first frame");
  if(auto* frequency=findControl<SkullKnob>(editor)) {
    frequency->setValue(7300.,juce::sendNotificationSync);
    expect(closeTo(parameter(processor,"frequency"),7300.f),
           "Frequency knob host-parameter attachment");
    frequency->setValue(5860.,juce::sendNotificationSync);
  } else expect(false,"Missing real frequency knob");
  if(auto* threshold=findControl<ThresholdFader>(editor)) {
    threshold->setValue(-35.,juce::sendNotificationSync);
    expect(closeTo(parameter(processor,"threshold"),-35.f),
           "Threshold fader host-parameter attachment");
    threshold->setValue(-24.,juce::sendNotificationSync);
  } else expect(false,"Missing real threshold fader");
  const int startingSlot=processor.getSlot();
  expect(click(editor,"NEXT") && processor.getSlot()!=startingSlot,
         "Actual preset NEXT must switch A/B");
  expect(click(editor,"PREV") && processor.getSlot()==startingSlot,
         "Actual preset PREV must switch A/B back");
  return failures;
}
juce::StringArray layoutProblems(SkellyDeEsserAudioProcessorEditor& editor) {
  juce::StringArray problems;
  const auto bounds=editor.getLocalBounds();
  for(int i=0;i<editor.getNumChildComponents();++i) {
    const auto* c=editor.getChildComponent(i);
    if(!c->isVisible())continue;
    if(c->getWidth()<=0||c->getHeight()<=0)
      problems.add("Visible component has zero bounds: "+c->getName());
    if(!bounds.contains(c->getBounds()))
      problems.add("Component is outside rendered UI: "+c->getName());
  }
  return problems;
}
void energiseActualDSP(SkellyDeEsserAudioProcessor& processor) {
  juce::MidiBuffer midi;
  juce::AudioBuffer<float> input(2,256);
  constexpr double sr=48000.,pi=3.14159265358979323846;
  for(int block=0;block<70;++block) {
    for(int i=0;i<input.getNumSamples();++i) {
      const int sample=block*256+i;
      const double envelope=(block%14<8 ? .33 : .055);
      const float v=static_cast<float>(
        .15*std::sin(2*pi*180*sample/sr)
        +envelope*std::sin(2*pi*5860*sample/sr));
      input.setSample(0,i,v);
      input.setSample(1,i,v*.8f);
    }
    processor.processBlock(input,midi);
  }
}
bool contactSheet(const juce::File& directory) {
  constexpr int cols=3,rows=4,cellW=440,cellH=330;
  const std::array<const char*,12> ids {{
    "minimum_split","minimum_phrase","compact_split",
    "compact_wide","compact_phrase","compact_sidechain",
    "compact_hpf","compact_preset_b","compact_disabled",
    "medium_phrase","large_split","maximum_split"
  }};
  juce::Image sheet(juce::Image::RGB,cols*cellW,rows*cellH,true);
  // Complete graphics operations before passing the image to the PNG encoder.
  // On Windows Direct2D, writes may be deferred until Graphics destruction.
  {
  juce::Graphics g(sheet);
  g.fillAll(juce::Colour::fromRGB(12,18,13));
  for(size_t i=0;i<ids.size();++i) {
    const auto file=directory.getChildFile(juce::String(ids[i])+".png");
    auto png=juce::ImageFileFormat::loadFrom(file);
    if(!png.isValid())return false;
    const int x=static_cast<int>(i%cols)*cellW;
    const int y=static_cast<int>(i/cols)*cellH;
    g.setColour(juce::Colour::fromRGB(157,255,157));
    g.setFont(juce::Font(juce::FontOptions(16.f)));
    g.drawText(ids[i],x+8,y+4,cellW-16,22,juce::Justification::centredLeft);
    g.drawImageWithin(png,x+8,y+28,cellW-16,cellH-36,
                      juce::RectanglePlacement::centred);
  }
  }
  const auto file=directory.getChildFile("contact-sheet.png");
  if(!skellcore::ui::headless::writePng(sheet,file))return false;
  const auto saved=juce::ImageFileFormat::loadFrom(file);
  return saved.isValid() && saved.getPixelAt(0,0).getARGB()
         ==juce::Colour::fromRGB(12,18,13).getARGB();

}

bool captureActualPopup(SkellyDeEsserAudioProcessorEditor& editor,
                        const char* anchor,const juce::File& output,
                        const juce::String& stem) {
  // Native popup menus are distinct top-level JUCE Components. An offscreen
  // editor snapshot cannot contain them, so temporarily attach the *real*
  // editor to the desktop and capture the actual menu window.
  if(!editor.isOnDesktop())editor.addToDesktop(juce::ComponentPeer::windowIsTemporary);
  editor.setVisible(true);
  editor.setBounds(180,140,633,422);
  juce::PopupMenu::dismissAllActiveMenus();
  auto& desktop=juce::Desktop::getInstance();
  const int before=desktop.getNumComponents();
  if(!click(editor,anchor))return false;
  #if JUCE_MODAL_LOOPS_PERMITTED
    if(auto* mm=juce::MessageManager::getInstance())
      mm->runDispatchLoopUntil(60);
  #endif
  juce::Component* popup=nullptr;
  for(int i=desktop.getNumComponents()-1;i>=0;--i){
    auto* comp=desktop.getComponent(i);
    if(comp!=&editor && comp!=nullptr && comp->isVisible()
       && comp->getWidth()>120 && comp->getHeight()>45){
      popup=comp;
      break;
    }
  }
  if(popup==nullptr || desktop.getNumComponents()<=before) {
    std::fprintf(stderr,"No visible JUCE popup for %s\n",anchor);
    juce::PopupMenu::dismissAllActiveMenus();
    return false;
  }
  const auto menuImage=popup->createComponentSnapshot(popup->getLocalBounds(),true);
  const auto editorImage=editor.createComponentSnapshot(editor.getLocalBounds(),true);
  if(!menuImage.isValid()||!editorImage.isValid()) {
    juce::PopupMenu::dismissAllActiveMenus();
    return false;
  }
  constexpr int sheetWidth=1120,sheetHeight=740,editorX=180,editorY=90;
  juce::Image shot(juce::Image::RGB,sheetWidth,sheetHeight,true);
  {
    juce::Graphics g(shot);
    g.fillAll(juce::Colour::fromRGB(5,11,7));
    g.drawImageAt(editorImage,editorX,editorY);
    const auto offset=popup->getScreenPosition()-editor.getScreenPosition();
    g.drawImageAt(menuImage,editorX+offset.x,editorY+offset.y);
  }
  const bool saved=skellcore::ui::headless::writePng(
    shot,output.getChildFile(stem+".png"));
  juce::PopupMenu::dismissAllActiveMenus();
  #if JUCE_MODAL_LOOPS_PERMITTED
    if(auto* mm=juce::MessageManager::getInstance())
      mm->runDispatchLoopUntil(45);
  #endif
  return saved && menuImage.getWidth()>120 && menuImage.getHeight()>45;
}
bool popupContactSheet(const juce::File& output) {
  constexpr int cols=3,w=400,h=300;
  const std::array<const char*,3> scenes{{"popup-filter","popup-preset","popup-settings"}};
  juce::Image overview(juce::Image::RGB,cols*w,h,true);
  {
    juce::Graphics g(overview);
    g.fillAll(juce::Colour::fromRGB(5,11,7));
    for(int i=0;i<cols;++i){
      auto file=output.getChildFile(juce::String(scenes[(size_t)i])+".png");
      auto img=juce::ImageFileFormat::loadFrom(file);
      if(!img.isValid())return false;
      g.setColour(juce::Colour::fromRGB(164,246,153));
      g.setFont(juce::Font(juce::FontOptions(17.f)));
      g.drawText(scenes[(size_t)i],i*w+10,5,w-20,25,juce::Justification::centred);
      g.drawImageWithin(img,i*w+8,36,w-16,h-44,
                        juce::RectanglePlacement::centred);
    }
  }
  return skellcore::ui::headless::writePng(
    overview,output.getChildFile("popup-contact-sheet.png"));
}
} // namespace

int main(int argc,char** argv) {
  juce::ScopedJuceInitialiser_GUI gui;
  SkellyDeEsserAudioProcessor processor;
  processor.prepareToPlay(48000.,256);
  int failures=0;
  {
    SkellyDeEsserAudioProcessorEditor editor(processor);
    failures+=exerciseActualControls(processor,editor);
    skellcore::ui::headless::Options options;
    options.project="Skelly DeEsser / real JUCE VST3 editor";
    const std::array<std::pair<const char*,int>,5> sizes {{
      {"minimum",540},{"compact",633},{"medium",806},{"large",979},{"maximum",1536}
    }};
    for(const auto& size:sizes) {
      const int h=juce::roundToInt(size.second/1.5f);
      options.scenes.push_back({juce::String(size.first)+"_split",size.second,h,0,0,false});
      options.scenes.push_back({juce::String(size.first)+"_phrase",size.second,h,0,2,false});
    }
    for(const char* state:{"wide","sidechain","hpf","preset_b","disabled",
                           "threshold_low","threshold_high","frequency_high","keyboard_focus"}) {
      options.scenes.push_back({juce::String("compact_")+state,633,422,0,0,false});
    }
    options.minimumColours=18;
    options.mustDiffer={
      {"compact_split","compact_phrase"},
      {"compact_split","compact_wide"},
      {"compact_split","compact_sidechain"},
      {"compact_split","compact_threshold_low"},
      {"compact_threshold_low","compact_threshold_high"}
    };
    options.configure=[&](const skellcore::ui::headless::Scene& scene) {
      editor.setSize(scene.width,scene.height);
      auto id=scene.id;
      if(processor.getSlot()==1)click(editor,"PREV");
      click(editor,"Split");
      click(editor,"Audio");
      if(parameter(processor,"sidechainHpf")>.5f)click(editor,"HPF");
      if(auto* b=findButton(editor,"S Chain"))b->setEnabled(true);
      if(auto* knob=findControl<SkullKnob>(editor))
        knob->setValue(5860.,juce::sendNotificationSync);
      if(auto* slider=findControl<ThresholdFader>(editor))
        slider->setValue(-24.,juce::sendNotificationSync);
      if(id.contains("phrase"))click(editor,"Phrase");
      else if(id.contains("wide"))click(editor,"Wide");
      if(id.contains("sidechain"))click(editor,"S Chain");
      if(id.contains("hpf"))click(editor,"HPF");
      if(id.contains("preset_b"))click(editor,"NEXT");
      if(id.contains("disabled"))
        if(auto* button=findButton(editor,"S Chain"))button->setEnabled(false);
      if(id.contains("threshold_low"))
        if(auto* slider=findControl<ThresholdFader>(editor))
          slider->setValue(-55.,juce::sendNotificationSync);
      if(id.contains("threshold_high"))
        if(auto* slider=findControl<ThresholdFader>(editor))
          slider->setValue(-5.,juce::sendNotificationSync);
      if(id.contains("frequency_high"))
        if(auto* knob=findControl<SkullKnob>(editor))
          knob->setValue(15500.,juce::sendNotificationSync);
      if(id.contains("keyboard_focus"))
        if(auto* button=findButton(editor,"Phrase"))button->grabKeyboardFocus();
      // Drive the actual processing path, never substitute synthetic UI shapes.
      energiseActualDSP(processor);
      editor.repaint();
    };
    options.inspect=[&] {return layoutProblems(editor);};
    const juce::String path=argc>1?juce::String::fromUTF8(argv[1]):"ui-screenshots";
    const auto output=juce::File::getCurrentWorkingDirectory().getChildFile(path);
    auto report=skellcore::ui::headless::render(editor,output,options);
    for(const auto& issue:report.errors) {
      std::fprintf(stderr,"visual regression: %s\n",issue.toRawUTF8());++failures;
    }
    if(report.captured!=static_cast<int>(options.scenes.size())) {
      std::fprintf(stderr,"Expected %zu images, got %d\n",options.scenes.size(),report.captured);
      ++failures;
    }
    if(!contactSheet(output)) {
      std::fprintf(stderr,"Could not create contact sheet from actual captures\n");++failures;
    }
    for(const auto& menu:std::array<std::pair<const char*,const char*>,3>{{
      {"FILTER","popup-filter"},
      {"A: My Default Preset","popup-preset"},
      {"MENU","popup-settings"}
    }}){
      if(!captureActualPopup(editor,menu.first,output,menu.second)){
        std::fprintf(stderr,"Popup capture failed: %s\n",menu.second);
        ++failures;
      }
    }
    if(!popupContactSheet(output)) {
      std::fprintf(stderr,"Could not create actual popup contact sheet\n");
      ++failures;
    }
    std::printf("Skelly DeEsser: %d actual JUCE scenes, %d failures\n",
                report.captured,failures);
  }
  processor.releaseResources();
  return failures==0?0:1;
}
