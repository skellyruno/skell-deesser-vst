#pragma once
#include "Theme.h"
class SkellyLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 void setArt(const juce::Image& knob,const juce::Image& cap){knobImage=knob;capImage=cap;}
 SkellyLookAndFeel(){setColour(juce::PopupMenu::backgroundColourId,Theme::black);
  setColour(juce::PopupMenu::textColourId,Theme::label);
  setColour(juce::PopupMenu::highlightedBackgroundColourId,Theme::dim);
  setColour(juce::PopupMenu::highlightedTextColourId,Theme::value);
  setColour(juce::PopupMenu::headerTextColourId,Theme::bright);
  setColour(juce::TextEditor::textColourId,Theme::value);
  setColour(juce::TextEditor::backgroundColourId,Theme::black);
  setColour(juce::TextEditor::outlineColourId,Theme::neon);}
 juce::Font getPopupMenuFont() override { return Theme::font(15.f,false); }
 void getIdealPopupMenuItemSize(const juce::String& text,bool separator,int standardHeight,
                                int& idealWidth,int& idealHeight) override {
  juce::LookAndFeel_V4::getIdealPopupMenuItemSize(text,separator,standardHeight,
                                                  idealWidth,idealHeight);
  if(!separator) {
   idealHeight=juce::jlimit(25,32,idealHeight);
   idealWidth=juce::jmax(idealWidth,130);
  }
 }
 void drawPopupMenuBackground(juce::Graphics&g,int width,int height) override {
  g.fillAll(Theme::black);
  g.setColour(Theme::mid.withAlpha(.66f));
  g.drawRect(0,0,width,height,1);
 }
 void drawButtonBackground(juce::Graphics&g,juce::Button& b,const juce::Colour&,bool hover,bool down) override {
  auto r=b.getLocalBounds().toFloat().reduced(2);
  const bool enabled=b.isEnabled();
  const bool lit=enabled&&(b.getToggleState()||down);
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
  // Highlight keyboard focus at the compact 55% default window.
  if(!enabled) {
   g.setColour(Theme::black.withAlpha(.48f));
   g.fillRoundedRectangle(r,4.f);
  }
  if(enabled&&b.hasKeyboardFocus(true)){
   g.setColour(Theme::bright.withAlpha(.90f));
   g.drawRoundedRectangle(r.reduced(.5f),5.f,1.5f);
  }
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool,bool) override {
  auto text=b.getButtonText();
  auto bounds=b.getLocalBounds().reduced(3);
  g.setColour(!b.isEnabled()?Theme::dim : b.getToggleState()?juce::Colours::white:Theme::label);
  // All glyphs are drawn as JUCE vectors: no missing-font Unicode symbols.
  if(text=="PREV" || text=="NEXT" || text=="FILTER"){
   const float cx=b.getWidth()*.5f,cy=b.getHeight()*.5f;
   juce::Path arrow;const bool left=text=="PREV";
   const float dx=juce::jmax(2.5f,b.getWidth()*.17f);
   const float dy=juce::jmax(3.f,b.getHeight()*.22f);
   if(left){arrow.startNewSubPath(cx+dx,cy-dy);arrow.lineTo(cx-dx,cy);arrow.lineTo(cx+dx,cy+dy);}
   else {arrow.startNewSubPath(cx-dx,cy-dy);arrow.lineTo(cx+dx,cy);arrow.lineTo(cx-dx,cy+dy);}
   if(text=="FILTER"){
    arrow.clear();arrow.addTriangle(cx-juce::jmax(3.f,b.getWidth()*.12f),
      cy-juce::jmax(4.f,b.getHeight()*.12f),
      cx+juce::jmax(3.f,b.getWidth()*.12f),
      cy-juce::jmax(4.f,b.getHeight()*.12f),
      cx,cy+juce::jmax(3.f,b.getHeight()*.10f));
    g.fillPath(arrow); // no tiny caption below a 20-pixel arrow
   }
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
  if(text=="HP" || text=="BP"){
   // Scale the path to actual button bounds (the compact view is only 55%).
   const float left=b.getWidth()*.20f, right=b.getWidth()*.80f;
   const float low=b.getHeight()*.69f,high=b.getHeight()*.33f;
   juce::Path path;
   path.startNewSubPath(left,low);
   if(text=="HP") {
    path.lineTo(left+(right-left)*.25f,low);
    path.lineTo(left+(right-left)*.65f,high);
    path.lineTo(right,high);
   } else {
    path.lineTo(left+(right-left)*.2f,(low+high)*.5f);
    path.lineTo(left+(right-left)*.48f,high);
    path.lineTo(left+(right-left)*.76f,(low+high)*.5f);
    path.lineTo(right,low);
   }
   g.strokePath(path,juce::PathStrokeType(juce::jmax(1.4f,b.getWidth()*.045f)));
   return; // shape itself distinguishes HP from BP, no clipped caption
  }
  const bool preset=text.startsWith("A: ")||text.startsWith("B: ");
  const float fs=juce::jlimit(9.f,22.f,b.getHeight()*(preset?.48f:.51f));
  auto font=Theme::font(fs,true);
  if(preset) {
   // Long preset names should ellipsize, never shrink to unreadable glyphs.
   while(text.length()>4 && font.getStringWidthFloat(text)>bounds.getWidth())
     text=text.dropLastCharacters(2)+"\xE2\x80\xA6";
  }
  g.setFont(font);
  g.drawFittedText(text,bounds,juce::Justification::centred,1,.82f);
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
  constexpr int ticks=30;
  for(int i=0;i<=ticks;++i){const float a=start+(end-start)*i/(float)ticks;
   auto p1=juce::Point<float>(cx+std::cos(a)*r*1.16f,cy+std::sin(a)*r*1.16f);
   auto p2=juce::Point<float>(cx+std::cos(a)*r*1.26f,cy+std::sin(a)*r*1.26f);
   g.setColour(i<=prop*ticks?Theme::neon.withAlpha(.95f):Theme::dim);
   g.drawLine(juce::Line<float>(p1,p2),juce::jmax(.8f,r*.027f));}
  juce::Path arc;arc.addCentredArc(cx,cy,r,r,0,start,start+(end-start)*prop,true);
  g.setColour(Theme::neon.withAlpha(.87f));
  g.strokePath(arc,juce::PathStrokeType(juce::jmax(1.9f,r*.09f),
       juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  const auto current=start+(end-start)*prop;
  g.setColour(Theme::value);
  g.fillEllipse(cx+std::cos(current)*r*1.16f-r*.065f,
                cy+std::sin(current)*r*1.16f-r*.065f,r*.13f,r*.13f);
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
  // The center skull artwork stays fixed, while the pointer genuinely rotates.
  const float theta=start+(end-start)*prop;
  const auto p1=juce::Point<float>(cx+std::cos(theta)*r*.20f,cy+std::sin(theta)*r*.20f);
  const auto p2=juce::Point<float>(cx+std::cos(theta)*r*.82f,cy+std::sin(theta)*r*.82f);
  g.setColour(Theme::bright);
  g.drawLine(juce::Line<float>(p1,p2),juce::jmax(1.8f,w*.025f));
  g.setColour(Theme::neon);
  g.fillEllipse(p2.x-2.f,p2.y-2.f,4.f,4.f);
 }
private:
 juce::Image knobImage,capImage;
};
