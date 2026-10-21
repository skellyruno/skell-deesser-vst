#pragma once
#include "EnvelopeFollower.h"
#include "SidechainFilter.h"
#include "WaveformHistory.h"
#include <JuceHeader.h>
#include <array>
#include <atomic>
class DeEsserEngine {
public:
 struct Settings{float threshold=-24,frequency=5860,ratio=4,range=12,attack=2,release=85,link=100,output=0;
 int mode=0,filterType=2,monitor=0;bool hpf=true;};
 std::atomic<float> sidechainLevelDb{-60},gainReductionDb{0},inputPeak{0};
 WaveformHistory waveform;
 void prepare(double sr){rate=sr;waveform.prepare(sr);for(auto& f:detectors)f.prepare(sr);for(auto& f:envelopes)f.prepare(sr);
  outputSmooth.reset(sr,.015);thresholdSmooth.reset(sr,.010);
  modeSmooth.reset(sr,.008);monitorSmooth.reset(sr,.008);
  modeSmooth.setCurrentAndTargetValue(0.);monitorSmooth.setCurrentAndTargetValue(0.);
  outputSmooth.setCurrentAndTargetValue(1.0);thresholdSmooth.setCurrentAndTargetValue(-24.0);
  for(auto& ch:band)for(auto& f:ch)f.reset();
  lastFreq=-1.;}
 void reset(){for(auto& f:detectors)f.reset();for(auto& ch:band)for(auto& f:ch)f.reset();sidechainLevelDb=-60;gainReductionDb=0;inputPeak=0;
  outputSmooth.setCurrentAndTargetValue(1.);thresholdSmooth.setCurrentAndTargetValue(-24.);
  modeSmooth.setCurrentAndTargetValue(0.);monitorSmooth.setCurrentAndTargetValue(0.);}
 void process(juce::AudioBuffer<float>& buffer,const Settings& s){
  const auto channels=std::min(2,buffer.getNumChannels()), n=buffer.getNumSamples();if(channels==0||n<=0)return;
  if(std::abs(s.frequency-lastFreq)>.5 || s.filterType!=lastFilter||s.hpf!=lastHpf){
   for(auto&f:detectors)f.configure(s.frequency,s.filterType,s.hpf);
   const auto lo=std::max(300.,(double)s.frequency*.68),hi=std::min(rate*.44,(double)s.frequency*1.48);
   for(auto& c:band){c[0].set(Biquad::highpass,lo,.70710678,rate);c[1].set(Biquad::highpass,lo,.70710678,rate);
    c[2].set(Biquad::lowpass,hi,.70710678,rate);c[3].set(Biquad::lowpass,hi,.70710678,rate);}
   lastFreq=s.frequency;lastFilter=s.filterType;lastHpf=s.hpf;
  }
  for(auto& f:envelopes) f.update(s.attack,s.release);
  outputSmooth.setTargetValue(juce::Decibels::decibelsToGain(s.output));
  thresholdSmooth.setTargetValue(s.threshold);
  modeSmooth.setTargetValue(s.mode == 0 ? 0. : 1.);
  monitorSmooth.setTargetValue(s.monitor == 1 ? 1. : 0.);
  const auto link=juce::jlimit(0.f,1.f,s.link*.01f);
  double maxEnv=0,maxReduction=0,peak=0;
  for(int i=0;i<n;++i){
   const auto output=outputSmooth.getNextValue();
   const auto currentThreshold=thresholdSmooth.getNextValue();
   const auto wideBlend=modeSmooth.getNextValue();
   const auto listenBlend=monitorSmooth.getNextValue();
   double input[2]{},det[2]{},env[2]{};
   for(int c=0;c<channels;++c){input[c]=buffer.getSample(c,i);peak=std::max(peak,std::abs(input[c]));det[c]=detectors[c].process(input[c]);env[c]=envelopes[c].process(det[c]);}
   waveform.push((float)(channels==2 ? .5 * (input[0]+input[1]) : input[0]));
   const auto shared=channels==2?std::max(env[0],env[1]):env[0];
   for(int c=0;c<channels;++c){
    const auto detectorEnv=(1.-link)*env[c]+link*shared;
    const auto db=20.*std::log10(std::max(detectorEnv,1.e-8));
    maxEnv=std::max(maxEnv,detectorEnv);
    // Continuous 6 dB-wide quadratic soft knee, conventional compressor transfer curve.
    constexpr double knee=6.;const auto over=db-currentThreshold;
    double reduction=0;
    if(over<=-knee/2) reduction=0;
    else if(over>=knee/2)reduction=(1.-1./std::max(1.f,s.ratio))*over;
    else reduction=(1.-1./std::max(1.f,s.ratio))*std::pow(over+knee/2,2)/(2*knee);
    reduction=std::clamp(reduction,0.,(double)s.range);maxReduction=std::max(maxReduction,reduction);
    const auto gain=std::pow(10.,-reduction/20.);
    double y=input[c];
     // Both branches always run: switching Split/Wide is smoothly interpolated.
    auto sb=input[c];for(auto&f:band[c])sb=f.process(sb);
    const auto split = input[c] + (gain-1.)*sb;
    const auto wide = input[c]*gain;
    y = split + (wide-split)*wideBlend;
    buffer.setSample(c,i,(float)((y+(det[c]-y)*listenBlend)*output));
   }
  }
  sidechainLevelDb.store((float)std::clamp(20.*std::log10(std::max(maxEnv,1.e-8)),-60.,0.));
  gainReductionDb.store((float)maxReduction);inputPeak.store((float)peak);
 }
private:
 double rate=48000,lastFreq=-1;int lastFilter=-1;bool lastHpf=false;
 std::array<SidechainFilter,2> detectors;std::array<EnvelopeFollower,2> envelopes;
 std::array<std::array<Biquad,4>,2> band;
 juce::SmoothedValue<double,juce::ValueSmoothingTypes::Linear> outputSmooth,thresholdSmooth,modeSmooth,monitorSmooth;

};
