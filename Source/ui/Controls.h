#pragma once
#include "Theme.h"

class SectionHeader:public juce::Component {
public:
 explicit SectionHeader(juce::String text):title(std::move(text)){setBufferedToImage(true);}
 void paint(juce::Graphics&g) override{g.setColour(Theme::label);g.setFont(Theme::font(20,true));
  g.drawText(title,getLocalBounds().withTrimmedBottom(10),juce::Justification::centred);
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
 setFont(Theme::font(24,true));setEditable(false,true,false);}};
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
  const auto w = (float)getWidth(), mid = getHeight() * .5f;
  g.setColour(Theme::mid.withAlpha(.35f));
  for (unsigned i = 0; i < values.size(); ++i) {
   const float x = w * (i + .5f) / values.size();
   const float amplitude = juce::jlimit(1.f, mid*.85f, values[i]*mid*.85f);
   g.drawLine(x, mid-amplitude, x, mid+amplitude, 1.5f);
  }
 }
 void advance(){repaint();}
private:
 const WaveformHistory& history;
};
class PresetBar:public juce::Component {
public:
 juce::TextButton previous{"◀"},next{"▶"},save{"▣"},menu{"☰"},name{"A: My Default Preset"};
 PresetBar(){for(auto*b:{&previous,&next,&save,&menu,&name})addAndMakeVisible(*b);}
 void resized()override {auto w=getWidth();previous.setBounds(0,0,48,50);name.setBounds(54,0,w-162,50);
  next.setBounds(w-104,0,48,50);save.setBounds(w-51,0,48,50);}
};
