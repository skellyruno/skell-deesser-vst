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
  auto text=b.getButtonText();
  auto bounds=b.getLocalBounds().reduced(3);
  g.setColour(b.getToggleState()?Theme::value:Theme::label);
  // All glyphs are drawn as JUCE vectors: no missing-font Unicode symbols.
  if(text=="PREV" || text=="NEXT" || text=="FILTER"){
   const float cx=b.getWidth()*.5f,cy=b.getHeight()*.5f;
   juce::Path arrow;const bool left=text=="PREV";
   if(left){arrow.startNewSubPath(cx+7,cy-10);arrow.lineTo(cx-7,cy);arrow.lineTo(cx+7,cy+10);}
   else {arrow.startNewSubPath(cx-7,cy-10);arrow.lineTo(cx+7,cy);arrow.lineTo(cx-7,cy+10);}
   if(text=="FILTER"){arrow.clear();arrow.addTriangle(cx-11,cy-5,cx+11,cy-5,cx,cy+8);g.fillPath(arrow);}
   else g.strokePath(arrow,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
   return;
  }
  if(text=="MENU"){
   const float x=b.getWidth()*.28f,w=b.getWidth()*.44f;
   for(int i=-1;i<=1;++i)g.drawLine(x,b.getHeight()*.5f+i*7,x+w,b.getHeight()*.5f+i*7,2.4f);
   return;
  }
  if(text=="SAVE"){
   auto r=b.getLocalBounds().toFloat().withSizeKeepingCentre(24.f,25.f);
   g.drawRoundedRectangle(r,1.5f,2.f);g.fillRect(r.getX()+5,r.getY()+3,13.f,7.f);
   g.drawRect(r.getX()+5,r.getY()+14,14.f,9.f,1.6f);return;
  }
  if(text=="HPF"){
   juce::Path path; const float x=b.getWidth()*.28f,y=b.getHeight()*.69f;
   path.startNewSubPath(x,y);path.lineTo(x+11,y);path.lineTo(x+29,y-19);path.lineTo(x+47,y-19);
   g.strokePath(path,juce::PathStrokeType(2.5f));return;
  }
  const bool preset=text.contains("Default Preset")||text.startsWith("A: ")||text.startsWith("B: ");
  g.setFont(Theme::font(juce::jmin(preset?17.f:18.f,b.getHeight()*.42f),true));
  g.drawFittedText(text,bounds,juce::Justification::centred,1,.57f);
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float, float,float,juce::Slider::SliderStyle,juce::Slider& slider) override {
  // Do not paint a black backing plate over the housing and live meter.
  const float top=juce::jmax(22.f,h*.048f),bottom=(float)h-top;
  const float pos=bottom-(float)slider.valueToProportionOfLength(slider.getValue())*(bottom-top);
  const float capW=juce::jmin(w*.95f,h*.23f), capH=capW*.81f;
  const float cx=(float)x+w*.5f,cy=(float)y+pos;
  if(capImage.isValid()){
   const float px=cx-capW*.5f,py=cy-capH*.5f;
   g.drawImage(capImage,px,py,capW,capH,0,0,capImage.getWidth(),capImage.getHeight());
  }else{
   auto r=juce::Rectangle<float>(cx-capW*.5f,cy-capH*.5f,capW,capH);
   Theme::panel(g,r,true);Theme::glow(g,r);
  }
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
