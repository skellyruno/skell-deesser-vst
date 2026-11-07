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
  // Strong, clearly visible selected state on the black/green metal panels.
  if(lit){
   juce::ColourGradient selected(Theme::mid.withAlpha(.85f),r.getCentreX(),r.getY(),
                                  Theme::dim.withAlpha(.96f),r.getCentreX(),r.getBottom(),false);
   g.setGradientFill(selected);
   g.fillRoundedRectangle(r.reduced(2),4);
   g.setColour(Theme::bright);
   g.drawRoundedRectangle(r.reduced(2),juce::jmax(2.f,b.getHeight()*.10f),1.1f);
  }else if(hover){
   g.setColour(Theme::neon.withAlpha(.10f));g.fillRoundedRectangle(r.reduced(2),4);
  }
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool,bool) override {
  auto text=b.getButtonText();
  auto bounds=b.getLocalBounds().reduced(3);
  g.setColour(b.getToggleState()?juce::Colours::white:Theme::label);
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
   const float x=b.getWidth()*.26f,w=b.getWidth()*.48f;
   const float gap=juce::jmax(3.f,b.getHeight()*.13f);
   for(int i=-1;i<=1;++i)
    g.drawLine(x,b.getHeight()*.5f+i*gap,x+w,b.getHeight()*.5f+i*gap,
               juce::jmax(1.2f,b.getWidth()*.055f));
   return;
  }
  if(text=="SAVE"){
   const float side=juce::jmin(b.getWidth(),b.getHeight())*.60f;
   auto r=b.getLocalBounds().toFloat().withSizeKeepingCentre(side,side*1.06f);
   const float u=side/24.f;
   g.drawRoundedRectangle(r,1.5f*u,juce::jmax(1.f,2.f*u));
   g.fillRect(r.getX()+5*u,r.getY()+3*u,13*u,7*u);
   g.drawRect(r.getX()+5*u,r.getY()+14*u,14*u,9*u,juce::jmax(1.f,1.6f*u));
   return;
  }
  if(text=="HPF"){
   // Scale the path to actual button bounds (the compact view is only 55%).
   const float left=b.getWidth()*.20f, right=b.getWidth()*.80f;
   const float low=b.getHeight()*.69f,high=b.getHeight()*.33f;
   juce::Path path;
   path.startNewSubPath(left,low);
   path.lineTo(left+(right-left)*.25f,low);
   path.lineTo(left+(right-left)*.65f,high);
   path.lineTo(right,high);
   g.strokePath(path,juce::PathStrokeType(juce::jmax(1.4f,b.getWidth()*.045f)));
   return;
  }
  const bool preset=text.contains("Default Preset")||text.startsWith("A: ")||text.startsWith("B: ");
  const float fs=juce::jlimit(9.f,22.f,b.getHeight()*(preset?.49f:.51f));
  g.setFont(Theme::font(fs,true));
  g.drawFittedText(text,bounds,juce::Justification::centred,1,.68f);
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
  else {
   // Draw a simple vector skull if artwork is missing (no unsupported font glyphs).
   g.setColour(Theme::value);g.fillEllipse(cx-r*.54f,cy-r*.6f,r*1.08f,r*.94f);
   g.fillRoundedRectangle(cx-r*.31f,cy+r*.05f,r*.62f,r*.38f,2.f);
   g.setColour(Theme::black);
   g.fillEllipse(cx-r*.39f,cy-r*.31f,r*.27f,r*.30f);
   g.fillEllipse(cx+r*.12f,cy-r*.31f,r*.27f,r*.30f);
   g.fillRect(cx-r*.08f,cy+r*.10f,r*.16f,r*.18f);
  }
 }
private:
 juce::Image knobImage,capImage;
};
