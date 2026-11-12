#pragma once
#include "Theme.h"
#include <cmath>
#include <functional>

class SectionHeader:public juce::Component {
public:
 explicit SectionHeader(juce::String text):title(std::move(text)){setBufferedToImage(true);}
 void paint(juce::Graphics&g) override{
  if(getWidth()<=0 || getHeight()<=0)return;
  // V6 used a fixed 20 px font and an 11 px star at every window size.
  // The heading could therefore be taller than its entire child component.
  const float fs=juce::jlimit(8.f,25.f,getHeight()*.53f);
  g.setColour(Theme::label);
  g.setFont(Theme::font(fs,true));
  const int decoration=juce::jmax(3,juce::roundToInt(getHeight()*.24f));
  g.drawFittedText(title,getLocalBounds().withTrimmedBottom(decoration),
                   juce::Justification::centred,1,0.85f);
  const float baseline=getHeight()-juce::jmax(1.f,getHeight()*.06f);
  Theme::line(g,3.f,baseline,getWidth()-3.f,baseline,.42f);
  const float starR=juce::jlimit(2.f,7.f,getHeight()*.14f);
  g.setColour(Theme::bright);
  juce::Path star;
  star.addStar({getWidth()*.5f,baseline},4,starR*.38f,starR);
  g.fillPath(star);
 }
private:juce::String title;
};
class TwoStateButton:public juce::TextButton {public:explicit TwoStateButton(const juce::String&t):juce::TextButton(t){setClickingTogglesState(false);setMouseCursor(juce::MouseCursor::PointingHandCursor);}};
class IconButton:public juce::TextButton {public:explicit IconButton(const juce::String&t):juce::TextButton(t){setMouseCursor(juce::MouseCursor::PointingHandCursor);}};
class ReadoutBox:public juce::Label {public:ReadoutBox(){setJustificationType(juce::Justification::centred);
 setColour(textColourId,Theme::value);setColour(backgroundColourId,Theme::black);
 setColour(outlineColourId,Theme::mid);setColour(textWhenEditingColourId,Theme::value);
 setColour(backgroundWhenEditingColourId,Theme::black);
 setColour(outlineWhenEditingColourId,Theme::bright);
 setFont(Theme::font(20,true));setMinimumHorizontalScale(0.72f);
 setEditable(true,true,false);
 setMouseCursor(juce::MouseCursor::IBeamCursor);
 }
 void setWheelStep(double step) noexcept {wheelStep=step;}
 std::function<void(double)> onWheelDelta;
 void mouseWheelMove(const juce::MouseEvent& event,const juce::MouseWheelDetails& wheel) override {
  if(!onWheelDelta || wheel.deltaY==0.f) {juce::Label::mouseWheelMove(event,wheel);return;}
  onWheelDelta((wheel.deltaY>0.f?1.0:-1.0)*wheelStep);
 }
private:
 double wheelStep=1.0;
};
class SkullKnob: public juce::Slider {public:SkullKnob(){
 setSliderStyle(RotaryHorizontalVerticalDrag);setTextBoxStyle(NoTextBox,false,0,0);
 setMouseCursor(juce::MouseCursor::PointingHandCursor);
 setDoubleClickReturnValue(true,5860);
 setScrollWheelEnabled(true);
 setMouseDragSensitivity(240);
}};
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
  if(w<=0.f || mid<=0.f)return;
  const float scale=w/1064.f;
  // Preserve a faint zero baseline during silence; never draw a fake signal.
  g.setColour(Theme::mid.withAlpha(.11f));
  g.drawLine(0.f,mid,w,mid,1.f);
  // Temporal smoothing is VISUAL only: the actual detector and waveform
  // history remain unchanged. Five-point weighted peaks remove harsh
  // square bars without introducing audio-thread work or allocations.
  std::array<float,WaveformHistory::length> heights{};
  for(int i=0;i<(int)values.size();++i) {
    auto peak=[&](int j)->float {
      return juce::jlimit(0.f,1.f,values[(size_t)juce::jlimit(0,(int)values.size()-1,j)]);
    };
    const float smoothed=(peak(i-2)+2.f*peak(i-1)+3.f*peak(i)
                         +2.f*peak(i+1)+peak(i+2))/9.f;
    heights[(size_t)i]=std::sqrt(smoothed)*mid*.53f;
  }
  // Two independent segments leave the threshold rail and skull fader clear.
  const float railCentre=371.f*scale,railHalf=92.f*scale;
  const int cutLeft=juce::jlimit(0,(int)values.size(),
                 (int)std::floor((railCentre-railHalf)/w*values.size()));
  const int cutRight=juce::jlimit(0,(int)values.size(),
                  (int)std::ceil((railCentre+railHalf)/w*values.size()));
  auto drawSegment=[&](int begin,int end) {
    if(end-begin<2)return;
    float maximum=0.f;
    juce::Path envelope,outline;
    for(int i=begin;i<end;++i) {
      const float x=w*(i+.5f)/values.size();
      const float h=heights[(size_t)i];
      maximum=juce::jmax(maximum,h);
      if(i==begin){
        envelope.startNewSubPath(x,mid-h);
        outline.startNewSubPath(x,mid-h);
      }else{
        envelope.lineTo(x,mid-h);
        outline.lineTo(x,mid-h);
      }
    }
    if(maximum<.7f)return;
    for(int i=end-1;i>=begin;--i)
      envelope.lineTo(w*(i+.5f)/values.size(),mid+heights[(size_t)i]);
    envelope.closeSubPath();
    g.setColour(Theme::mid.withAlpha(.13f));
    g.fillPath(envelope);
    g.setColour(Theme::bright.withAlpha(.48f));
    g.strokePath(outline,juce::PathStrokeType(1.f));
  };
  drawSegment(0,cutLeft);
  drawSegment(cutRight,(int)values.size());
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
