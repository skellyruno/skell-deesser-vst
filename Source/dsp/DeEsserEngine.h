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
  rate = (std::isfinite(sr) && sr >= 1000.0 && sr <= 768000.0) ? sr : 48000.0;
  waveform.prepare(rate);
  for (auto& f:detectors) {f.prepare(rate);f.setModeSmoothing(6.0);}
  for (auto& f:envelopes)f.prepare(rate);
  for (auto& b:band)b.prepare(rate);
  phrase.prepare(rate);
  phraseAttack = std::exp(-1.0 / (rate * .004));
  phraseRelease = std::exp(-1.0 / (rate * .026));
  outputSmooth.reset(rate,.015);
  thresholdSmooth.reset(rate,.010);
  // Host automation can jump from a mild de-ess setting to a full 24 dB
  // reduction between audio blocks. Interpolate the parameters instead of
  // creating a gain discontinuity at each block boundary.
  ratioSmooth.reset(rate,.012);
  rangeSmooth.reset(rate,.012);
  linkSmooth.reset(rate,.010);
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
  ratioSmooth.setCurrentAndTargetValue(4.0);
  rangeSmooth.setCurrentAndTargetValue(12.0);
  linkSmooth.setCurrentAndTargetValue(1.0);
  modeSmooth.setCurrentAndTargetValue(0.0);
  phraseModeSmooth.setCurrentAndTargetValue(0.0);
  monitorSmooth.setCurrentAndTargetValue(0.0);
  currentFreq=5860.0;
  filterCounter=0;
  lastConfiguredFreq=-1.0;
  lastFilterType=-1;
  lastHpf=false;
  filterReconfigurations=0;
  meterSideDb=-60.0;
  meterGr=0.0;
  firstAudioBlock=true;
 }
 void process(juce::AudioBuffer<float>& buffer,const Settings& s) {
  const int channels=std::min(2,buffer.getNumChannels());
  const int n=buffer.getNumSamples();
  if(channels<=0||n<=0)return;
  // Malformed host states/automation may contain NaN or infinity.
  // Sanitize once per block and initialize from the saved preset on first use.
  const Settings safe=sanitized(s);
  if(firstAudioBlock){
   outputSmooth.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(safe.output));
   thresholdSmooth.setCurrentAndTargetValue(safe.threshold);
   ratioSmooth.setCurrentAndTargetValue(safe.ratio);
   rangeSmooth.setCurrentAndTargetValue(safe.range);
   linkSmooth.setCurrentAndTargetValue(safe.link*.01);
   modeSmooth.setCurrentAndTargetValue(safe.mode==1 ? 1.0 : 0.0);
   phraseModeSmooth.setCurrentAndTargetValue(safe.mode==2 ? 1.0 : 0.0);
   monitorSmooth.setCurrentAndTargetValue(safe.monitor==1 ? 1.0 : 0.0);
   currentFreq=std::clamp((double)safe.frequency,70.0,rate*.30);
   lastConfiguredFreq=-1.0;
   firstAudioBlock=false;
  }
  for(auto& f:envelopes)f.update(safe.attack,safe.release);
  outputSmooth.setTargetValue(juce::Decibels::decibelsToGain(safe.output));
  thresholdSmooth.setTargetValue(safe.threshold);
  ratioSmooth.setTargetValue(safe.ratio);
  rangeSmooth.setTargetValue(safe.range);
  linkSmooth.setTargetValue(safe.link*.01);
  modeSmooth.setTargetValue(safe.mode==1?1.0:0.0);
  phraseModeSmooth.setTargetValue(safe.mode==2?1.0:0.0);
  monitorSmooth.setTargetValue(safe.monitor==1?1.0:0.0);
  const double targetFreq=std::clamp((double)safe.frequency,70.0,rate*.30);
  const double freqAlpha=1.0-std::exp(-1.0/(rate*.020));
  double maxEnv=0.0,maxReduction=0.0,peak=0.0;
  float lastPhraseBaseline=-60.f;int lastPhraseEvents=0;
  for(int i=0;i<n;++i) {
   currentFreq += (targetFreq-currentFreq)*freqAlpha;
   // At a maximum 16-sample cadence, only calculate trig-heavy biquad
   // coefficients when the requested setting actually changed. Static
   // parameters used to recompute eight biquads thousands of times/second.
   // The 0.02% cutoff tolerance is below perceptual tuning resolution and
   // bounds coefficient work during 20 ms frequency sweeps.
   if ((filterCounter++ & 15u)==0u &&
       (lastFilterType!=safe.filterType || lastHpf!=safe.hpf ||
        std::abs(currentFreq-lastConfiguredFreq)>std::max(.20,currentFreq*.0002))) {
    for(auto& f:detectors)f.configure(currentFreq,safe.filterType,safe.hpf);
    for(auto& b:band)b.setFrequency(currentFreq);
    lastConfiguredFreq=currentFreq;
    lastFilterType=safe.filterType;
    lastHpf=safe.hpf;
    ++filterReconfigurations;
   }
   const double output=outputSmooth.getNextValue();
   const double currentThreshold=thresholdSmooth.getNextValue();
   const double ratio=ratioSmooth.getNextValue();
   const double range=rangeSmooth.getNextValue();
   const double link=linkSmooth.getNextValue();
   const double wideBlend=modeSmooth.getNextValue();
   const double phraseBlend=phraseModeSmooth.getNextValue();
   const double listenBlend=monitorSmooth.getNextValue();
   double input[2]{},det[2]{},env[2]{};
   double inputAbs=0.0, detectorAbs=0.0;
   for(int c=0;c<channels;++c) {
    input[c] = buffer.getSample(c,i);
    if(!std::isfinite(input[c]))input[c]=0.0;
    input[c]=std::clamp(input[c],-1.0e6,1.0e6);
    peak=std::max(peak,std::abs(input[c]));
    inputAbs=std::max(inputAbs,std::abs(input[c]));
    det[c]=detectors[c].process(input[c]);
    detectorAbs=std::max(detectorAbs,std::abs(det[c]));
    env[c]=envelopes[c].process(det[c]);
   }
   waveform.push((float)peakForFrame(input,channels));
   const double shared=channels==2?std::max(env[0],env[1]):env[0];
   const auto phraseDecision=phrase.process(inputAbs,detectorAbs,currentThreshold,safe.phraseSensitivity);
   lastPhraseBaseline=(float)phraseDecision.baselineDb;
   lastPhraseEvents=phraseDecision.learnedEvents;
   for(int c=0;c<channels;++c) {
    const double detectorEnv=(1.0-link)*env[c]+link*shared;
    const double db=20.0*std::log10(std::max(detectorEnv,1.e-8));
    maxEnv=std::max(maxEnv,detectorEnv);
    const double reduction=computeReduction(db,currentThreshold,ratio,range);
    // Phrase-Aware uses the previous sibilant events within this phrase as
    // its reference; first event is learned, not automatically attenuated.
    const double targetPhraseReduction=phraseDecision.eligible
      ? computeReduction(db,phraseDecision.thresholdDb,ratio,range) : 0.0;
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
    const double result=(processed+(det[c]-processed)*listenBlend)*output;
    buffer.setSample(c,i,(float)(std::isfinite(result)
                       ? std::clamp(result,-1.0e6,1.0e6) : 0.0));
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
 // Inspection-only counter used by DSP regression tests; never read it
 // concurrently with process() on the realtime thread.
 unsigned getFilterReconfigurationCount() const noexcept { return filterReconfigurations; }
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
 static Settings sanitized(Settings s) noexcept {
  auto bounded=[](float v,float lo,float hi,float fallback) noexcept {
   return std::isfinite(v) ? std::clamp(v,lo,hi) : fallback;
  };
  s.threshold=bounded(s.threshold,-60.f,0.f,-24.f);
  s.frequency=bounded(s.frequency,1000.f,20000.f,5860.f);
  s.ratio=bounded(s.ratio,1.f,20.f,4.f);
  s.range=bounded(s.range,0.f,24.f,12.f);
  s.attack=bounded(s.attack,.1f,30.f,2.f);
  s.release=bounded(s.release,10.f,500.f,85.f);
  s.link=bounded(s.link,0.f,100.f,100.f);
  s.output=bounded(s.output,-12.f,12.f,0.f);
  s.phraseSensitivity=bounded(s.phraseSensitivity,1.f,12.f,5.f);
  s.mode=std::clamp(s.mode,0,2);
  s.filterType=std::clamp(s.filterType,0,4);
  s.monitor=std::clamp(s.monitor,0,1);
  return s;
 }
 static double peakForFrame(const double* samples,int channels) noexcept {
  return channels==2?std::max(std::abs(samples[0]),std::abs(samples[1])):std::abs(samples[0]);
 }
 double rate=48000.0,currentFreq=5860.0,meterSideDb=-60.0,meterGr=0.0;
 unsigned filterCounter=0,filterReconfigurations=0;
 double lastConfiguredFreq=-1.0;
 int lastFilterType=-1;
 bool lastHpf=false,firstAudioBlock=true;
 std::array<SidechainFilter,2> detectors;
 std::array<EnvelopeFollower,2> envelopes;
 std::array<ComplementaryBand,2> band;
 PhraseAwareDetector phrase;
 std::array<double,2> phraseReduction{{0.0,0.0}};
 double phraseAttack=0.99,phraseRelease=0.999;
 juce::SmoothedValue<double,juce::ValueSmoothingTypes::Linear> outputSmooth,thresholdSmooth,ratioSmooth,rangeSmooth,linkSmooth,
  modeSmooth,phraseModeSmooth,monitorSmooth;
};
