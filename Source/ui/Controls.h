#pragma once
#include "Theme.h"
#include <cmath>

class SectionHeader:public juce::Component {
public:
 explicit SectionHeader(juce::String text):title(std::move(text)){setBufferedToImage(true);}
 void paint(juce::Graphics&g) override{g.setColour(Theme::label);g.setFont(Theme::font(20,true));
  g.drawFittedText(title,getLocalBounds().withTrimmedBottom(juce::jmax(3,getHeight()/5)),juce::Justification::centred,1,0.7f);
  const float y=getHeight()-3.f;Theme::line(g,0,y,getWidth(),y,.6f);
  g.setColour(Theme::neon);juce::Path star;star.addStar({getWidth()*.5f,y},4,3,11);g.fillPath(star);
 }
private:juce::String title;
};
class TwoStateButton:public juce::TextButton {public:explicit TwoStateButton(const juce::String&t):juce::TextButton(t){setClickingTogglesState(false);setMouseCursor(juce::MouseCursor::PointingHandCursor);}};
class IconButton:public juce::TextButton {public:explicit IconButton(const juce::String&t):juce::TextButton(t){setMouseCursor(juce::MouseCursor::PointingHandCursor);}};
class ReadoutBox:public juce::Label {public:ReadoutBox(){setJustificationType(juce::Justification::centred);
 setColour(textColourId,Theme::value);setColour(backgroundColourId,Theme::black);
 setColour(outlineColourId,Theme::mid);setColour(textWhenEditingColourId,Theme::value);
 setFont(Theme::font(20,true));setMinimumHorizontalScale(0.6f);setEditable(false,true,false);}};
class SkullKnob: public juce::Slider {public:SkullKnob(){setSliderStyle(RotaryHorizontalVerticalDrag);setTextBoxStyle(NoTextBox,false,0,0);
 setMouseCursor(juce::MouseCursor::PointingHandCursor);setDoubleClickReturnValue(true,5860);}};
class ThresholdFader:public juce::Slider {public:ThresholdFader(){setSliderStyle(LinearVertical);setTextBoxStyle(NoTextBox,false,0,0);
 setDoubleClickReturnValue(true,-24.);setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
 setScrollWheelEnabled(true);}};
class WaveformBackdrop:public juce::Component {
public:
 explicit WaveformBackdrop(const WaveformHistory& h):history(h){setInterceptsMouseClicks(false,false);}
 void paint(juce::Graphics& g) override {
  std::array<float, WaveformHistory::length> values{};
  history.copy(values.data(), static_cast<unsigned>(values.size()));
  const float w=(float)getWidth(),mid=getHeight()*.5f;
  const float scale=w/1064.f;
  // Leave an unobstructed gap for the threshold rail in the middle of the stage.
  const float railCentre=371.f*scale,railHalf=92.f*scale;
  for(unsigned i=0;i<values.size();++i){
   const float x=w*(i+.5f)/values.size();
   if(std::abs(x-railCentre)<railHalf)continue;
   const float peak=juce::jlimit(0.f,1.f,values[i]);
   const float amplitude=juce::jmax(1.f,std::sqrt(peak)*mid*.78f);
   g.setColour(Theme::mid.withAlpha(.21f));
   g.drawLine(x,mid-amplitude-2.f,x,mid+amplitude+2.f,3.f);
   g.setColour(Theme::bright.withAlpha(.36f));
   g.drawLine(x,mid-amplitude,x,mid+amplitude,1.f);
  }
 }
 void advance(){repaint();}
private:
 const WaveformHistory& history;
};
class PresetBar:public juce::Component {
public:
 juce::TextButton previous{"PREV"},next{"NEXT"},save{"SAVE"},menu{"MENU"},name{"A: My Default Preset"};
 PresetBar(){for(auto*b:{&previous,&next,&save,&menu,&name})addAndMakeVisible(*b);}
 void resized()override {auto w=getWidth();previous.setBounds(0,0,48,50);name.setBounds(54,0,w-162,50);
  next.setBounds(w-104,0,48,50);save.setBounds(w-51,0,48,50);}
};
