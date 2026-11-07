#pragma once
#include "ComplementaryBand.h"
#include "EnvelopeFollower.h"
#include "SidechainFilter.h"
#include "WaveformHistory.h"
#include "PhraseAwareDetector.h"
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>

class DeEsserEngine {
public:
 struct Settings {
  float threshold=-24,frequency=5860,ratio=4,range=12,attack=2,release=85,link=100,output=0,phraseSensitivity=5;
  int mode=0,filterType=2,monitor=0;
  bool hpf=true;
 };
 std::atomic<float> sidechainLevelDb{-60},gainReductionDb{0},inputPeak{0},phraseBaselineDb{-60};
 std::atomic<int> phraseLearnedEvents{0};
 WaveformHistory waveform;
 void prepare(double sr) {
  rate = std::max(1000.0,sr);
  waveform.prepare(rate);
  for (auto& f:detectors) {f.prepare(rate);f.setModeSmoothing(6.0);}
  for (auto& f:envelopes)f.prepare(rate);
  for (auto& b:band)b.prepare(rate);
  phrase.prepare(rate);
  phraseAttack = std::exp(-1.0 / (rate * .004));
  phraseRelease = std::exp(-1.0 / (rate * .026));
  outputSmooth.reset(rate,.015);
  thresholdSmooth.reset(rate,.010);
  modeSmooth.reset(rate,.008);
  phraseModeSmooth.reset(rate,.008);
  monitorSmooth.reset(rate,.008);
  reset();
 }
 void reset() {
  for (auto& f:detectors)f.reset();
  for (auto& f:envelopes)f.prepare(rate);
  for (auto& b:band)b.reset();
  phrase.reset();
  phraseReduction = {0.0,0.0};
  phraseBaselineDb.store(-60);phraseLearnedEvents.store(0);
  sidechainLevelDb.store(-60);gainReductionDb.store(0);inputPeak.store(0);
  outputSmooth.setCurrentAndTargetValue(1.0);
  thresholdSmooth.setCurrentAndTargetValue(-24.0);
  modeSmooth.setCurrentAndTargetValue(0.0);
  phraseModeSmooth.setCurrentAndTargetValue(0.0);
  monitorSmooth.setCurrentAndTargetValue(0.0);
  currentFreq=5860.0;
  filterCounter=0;
  meterSideDb=-60.0;
  meterGr=0.0;
 }
 void process(juce::AudioBuffer<float>& buffer,const Settings& s) {
  const int channels=std::min(2,buffer.getNumChannels());
  const int n=buffer.getNumSamples();
  if(channels<=0||n<=0)return;
  for(auto& f:envelopes)f.update(s.attack,s.release);
  outputSmooth.setTargetValue(juce::Decibels::decibelsToGain(s.output));
  thresholdSmooth.setTargetValue(s.threshold);
  modeSmooth.setTargetValue(s.mode==1?1.0:0.0);
  phraseModeSmooth.setTargetValue(s.mode==2?1.0:0.0);
  monitorSmooth.setTargetValue(s.monitor==1?1.0:0.0);
  const double link=std::clamp(s.link*.01,0.0,1.0);
  const double targetFreq=std::clamp((double)s.frequency,70.0,rate*.30);
  const double freqAlpha=1.0-std::exp(-1.0/(rate*.020));
  double maxEnv=0.0,maxReduction=0.0,peak=0.0;
  float lastPhraseBaseline=-60.f;int lastPhraseEvents=0;
  for(int i=0;i<n;++i) {
   currentFreq += (targetFreq-currentFreq)*freqAlpha;
   // Update coefficient targets at a bounded 16-sample cadence. The frequency
   // itself is smoothed sample-by-sample (20 ms); no allocation in audio thread.
   if ((filterCounter++ & 15u)==0u) {
    for(auto& f:detectors)f.configure(currentFreq,s.filterType,s.hpf);
    for(auto& b:band)b.setFrequency(currentFreq);
   }
   const double output=outputSmooth.getNextValue();
   const double currentThreshold=thresholdSmooth.getNextValue();
   const double wideBlend=modeSmooth.getNextValue();
   const double phraseBlend=phraseModeSmooth.getNextValue();
   const double listenBlend=monitorSmooth.getNextValue();
   double input[2]{},det[2]{},env[2]{};
   double inputAbs=0.0, detectorAbs=0.0;
   for(int c=0;c<channels;++c) {
    input[c] = buffer.getSample(c,i);
    if(!std::isfinite(input[c]))input[c]=0.0;
    peak=std::max(peak,std::abs(input[c]));
    inputAbs=std::max(inputAbs,std::abs(input[c]));
    det[c]=detectors[c].process(input[c]);
    detectorAbs=std::max(detectorAbs,std::abs(det[c]));
    env[c]=envelopes[c].process(det[c]);
   }
   waveform.push((float)peakForFrame(input,channels));
   const double shared=channels==2?std::max(env[0],env[1]):env[0];
   const auto phraseDecision=phrase.process(inputAbs,detectorAbs,currentThreshold,s.phraseSensitivity);
   lastPhraseBaseline=(float)phraseDecision.baselineDb;
   lastPhraseEvents=phraseDecision.learnedEvents;
   for(int c=0;c<channels;++c) {
    const double detectorEnv=(1.0-link)*env[c]+link*shared;
    const double db=20.0*std::log10(std::max(detectorEnv,1.e-8));
    maxEnv=std::max(maxEnv,detectorEnv);
    const double reduction=computeReduction(db,currentThreshold,s.ratio,s.range);
    // Phrase-Aware uses the previous sibilant events within this phrase as
    // its reference; first event is learned, not automatically attenuated.
    const double targetPhraseReduction=phraseDecision.eligible
      ? computeReduction(db,phraseDecision.thresholdDb,s.ratio,s.range) : 0.0;
    const double smoothing=targetPhraseReduction>phraseReduction[(size_t)c]
      ? phraseAttack : phraseRelease;
    phraseReduction[(size_t)c]=smoothing*phraseReduction[(size_t)c]
      +(1.0-smoothing)*targetPhraseReduction;
    maxReduction=std::max(maxReduction,reduction*(1.0-phraseBlend)
                          +phraseReduction[(size_t)c]*phraseBlend);
    const double gain=std::pow(10.0,-reduction/20.0);
    // Parallel subtraction: 0 dB GR is EXACT dry, even while sweeping cutoff.
    // The complementary splitter guarantees mid+low+high = input each sample.
    const double sibilance=band[c].process(input[c]);
    const double split=input[c]+(gain-1.0)*sibilance;
    const double wide=input[c]*gain;
    const double phraseGain=std::pow(10.0,-phraseReduction[(size_t)c]/20.0);
    const double phraseSplit=input[c]+(phraseGain-1.0)*sibilance;
    // Two independent smoothed controls may transition simultaneously (e.g.
    // host automates Split->Wide while enabling Phrase). Use a convex mix so
    // the weights can never sum above unity or invert the Split contribution.
    const double conventional=split+(wide-split)*wideBlend;
    const double processed=conventional*(1.0-phraseBlend)+phraseSplit*phraseBlend;
    buffer.setSample(c,i,(float)((processed+(det[c]-processed)*listenBlend)*output));
   }
  }
  // Ballistic display meters. Fast rise/slow fall prevents block-dependent GUI
  // flicker without changing envelope or audio compression timing.
  const double dt=n/rate;
  const double rise=std::exp(-dt/.014),fall=std::exp(-dt/.170);
  auto approach=[&](double& previous,double value) {
   const double k=value>previous?rise:fall;
   previous=k*previous+(1.0-k)*value;
  };
  approach(meterSideDb,std::clamp(20.0*std::log10(std::max(maxEnv,1.e-8)),-60.0,0.0));
  approach(meterGr,maxReduction);
  sidechainLevelDb.store((float)meterSideDb,std::memory_order_relaxed);
  gainReductionDb.store((float)meterGr,std::memory_order_relaxed);
  inputPeak.store((float)peak,std::memory_order_relaxed);
  phraseBaselineDb.store((float)lastPhraseBaseline,std::memory_order_relaxed);
  phraseLearnedEvents.store(lastPhraseEvents,std::memory_order_relaxed);
 }
 static double computeReduction(double db,double threshold,double ratio,double range) noexcept {
  constexpr double knee=6.0;
  const double slope=1.0-1.0/std::max(1.0,ratio);
  const double over=db-threshold;
  double reduction=0.0;
  if(over>=knee*.5)reduction=slope*over;
  else if(over>-knee*.5) {
   const double z=over+knee*.5;
   reduction=slope*z*z/(2.0*knee);
  }
  return std::clamp(reduction,0.0,std::max(0.0,range));
 }
private:
 static double peakForFrame(const double* samples,int channels) noexcept {
  return channels==2?std::max(std::abs(samples[0]),std::abs(samples[1])):std::abs(samples[0]);
 }
 double rate=48000.0,currentFreq=5860.0,meterSideDb=-60.0,meterGr=0.0;
 unsigned filterCounter=0;
 std::array<SidechainFilter,2> detectors;
 std::array<EnvelopeFollower,2> envelopes;
 std::array<ComplementaryBand,2> band;
 PhraseAwareDetector phrase;
 std::array<double,2> phraseReduction{{0.0,0.0}};
 double phraseAttack=0.99,phraseRelease=0.999;
 juce::SmoothedValue<double,juce::ValueSmoothingTypes::Linear> outputSmooth,thresholdSmooth,modeSmooth,phraseModeSmooth,monitorSmooth;
};
