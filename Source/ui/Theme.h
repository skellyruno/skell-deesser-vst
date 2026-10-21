#pragma once
#include <JuceHeader.h>
namespace Theme {
constexpr int width = 1536, height = 1024;
const juce::Colour black = juce::Colour::fromRGB(3,8,4);
const juce::Colour neon = juce::Colour::fromRGB(57,255,58);
const juce::Colour bright = juce::Colour::fromRGB(125,255,122);
const juce::Colour mid = juce::Colour::fromRGB(31,181,42);
const juce::Colour dim = juce::Colour::fromRGB(15,74,20);
const juce::Colour label = juce::Colour::fromRGB(157,255,157);
const juce::Colour value = juce::Colour::fromRGB(216,255,216);
constexpr juce::Rectangle<int> left{50,200,255,660};
constexpr juce::Rectangle<int> stage{350,215,1140,645};
constexpr juce::Rectangle<int> fader{703,297,132,474};
inline juce::Font font(float sz, bool bold=false) {return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(),sz,bold ? juce::Font::bold : juce::Font::plain));}
inline void line(juce::Graphics& g, float x1,float y1,float x2,float y2,float a=.6f) {
 g.setColour(neon.withAlpha(a)); g.drawLine(x1,y1,x2,y2,1.0f);
}
inline void panel(juce::Graphics& g, juce::Rectangle<float> r, bool active=false) {
 g.setColour(black.withAlpha(.94f)); g.fillRoundedRectangle(r,5.f);
 g.setColour((active ? neon : mid).withAlpha(active ? .95f : .75f));g.drawRoundedRectangle(r,5.f,1.4f);
}
inline void glow(juce::Graphics&g,juce::Rectangle<float>r) {
 for(int i=5;i>=1;--i){g.setColour(neon.withAlpha(.013f*i));g.drawRoundedRectangle(r.expanded((float)i),4.f+i,.9f);}
}
}
