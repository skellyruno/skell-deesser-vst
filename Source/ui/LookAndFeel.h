#pragma once
#include "Theme.h"
class SkellyLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 void setArt(const juce::Image& knob,const juce::Image& cap){knobImage=knob;capImage=cap;}
 SkellyLookAndFeel(){setColour(juce::PopupMenu::backgroundColourId,Theme::black);
  setColour(juce::PopupMenu::textColourId,Theme::label);
  setColour(juce::PopupMenu::highlightedBackgroundColourId,Theme::dim);
  setColour(juce::TextEditor::textColourId,Theme::value);
  setColour(juce::TextEditor::backgroundColourId,Theme::black);
  setColour(juce::TextEditor::outlineColourId,Theme::neon);}
 void drawButtonBackground(juce::Graphics&g,juce::Button& b,const juce::Colour&,bool hover,bool down) override {
  auto r=b.getLocalBounds().toFloat().reduced(2);
  const bool lit=b.getToggleState()||down;
  Theme::panel(g,r,lit);if(lit||hover)Theme::glow(g,r);
  if(lit){g.setColour(Theme::neon.withAlpha(.18f));g.fillRoundedRectangle(r.reduced(2),4);}
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool,bool) override {
  g.setColour(b.getToggleState()?Theme::value:Theme::label);g.setFont(Theme::font(juce::jmin(19.f,b.getHeight()*.42f),true));
  g.drawFittedText(b.getButtonText(),b.getLocalBounds(),juce::Justification::centred,1);
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float, float,float,juce::Slider::SliderStyle,juce::Slider& slider) override {
  const auto top=12.f,bottom=(float)h-12.f;
  const float pos=bottom-(float)slider.valueToProportionOfLength(slider.getValue())*(bottom-top);
  g.setColour(Theme::black);g.fillRoundedRectangle((float)x+w*.33f,(float)y+top,w*.34f,bottom-top,4);
  auto r=juce::Rectangle<float>((float)x+w*.12f,(float)y+pos-25.f,w*.76f,50.f);
  Theme::panel(g,r,true);Theme::glow(g,r);
  if(capImage.isValid())g.drawImage(capImage,r.expanded(7).getX(),r.expanded(7).getY(),r.expanded(7).getWidth(),r.expanded(7).getHeight(),0,0,capImage.getWidth(),capImage.getHeight());
  else {g.setColour(Theme::bright);g.setFont(Theme::font(24,true));g.drawFittedText("☠",r.toNearestInt(),juce::Justification::centred,1);}
 }
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float prop,float start,float end,juce::Slider&) override {
  const auto cx=x+w*.5f,cy=y+h*.5f,r=juce::jmin(w,h)*.38f;
  for(int i=0;i<48;++i){const float a=start+(end-start)*i/47.f;
   auto p1=juce::Point<float>(cx+std::cos(a)*r*1.14f,cy+std::sin(a)*r*1.14f);
   auto p2=juce::Point<float>(cx+std::cos(a)*r*1.26f,cy+std::sin(a)*r*1.26f);
   g.setColour(i<=prop*47?Theme::neon:Theme::dim);g.drawLine(juce::Line<float>(p1,p2),1.7f);}
  juce::Path arc;arc.addCentredArc(cx,cy,r,r,0,start,start+(end-start)*prop,true);
  g.setColour(Theme::neon);g.strokePath(arc,juce::PathStrokeType(5.f));
  g.setColour(juce::Colour::fromRGB(12,22,14));g.fillEllipse(cx-r*.9f,cy-r*.9f,r*1.8f,r*1.8f);
  g.setColour(Theme::bright);g.drawEllipse(cx-r*.9f,cy-r*.9f,r*1.8f,r*1.8f,1.2f);
  if(knobImage.isValid())g.drawImage(knobImage,cx-r*.9f,cy-r*.9f,r*1.8f,r*1.8f,0,0,knobImage.getWidth(),knobImage.getHeight());
  else {g.setFont(Theme::font(r*.87f,true));g.drawText("☠",juce::Rectangle<float>(cx-r*.7f,cy-r*.76f,r*1.4f,r*1.5f),juce::Justification::centred);}
 }
private:
 juce::Image knobImage,capImage;
};
