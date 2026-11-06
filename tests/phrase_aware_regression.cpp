#include "../Source/dsp/PhraseAwareDetector.h"
#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
constexpr double sr = 48000.0;
constexpr double pi = 3.14159265358979323846;

struct SegmentSummary {
 bool everEligible = false;
 bool everReady = false;
 double minThreshold = 120.0;
 double baselineAtEnd = -60.0;
 int learnedAtEnd = 0;
};
SegmentSummary push(PhraseAwareDetector& tracker, double voice, double detector, double durationMs) {
 SegmentSummary result;
 const int n = (int)std::lround(sr * durationMs / 1000.0);
 for (int i = 0; i < n; ++i) {
  auto d = tracker.process(voice, detector, -50.0, 5.0);
  result.everEligible |= d.eligible;
  result.everReady |= d.ready;
  result.minThreshold = std::min(result.minThreshold, d.thresholdDb);
  result.baselineAtEnd = d.baselineDb;
  result.learnedAtEnd = d.learnedEvents;
 }
 return result;
}

// Extract the 5.86 kHz component from the last part of a burst. Comparing
// projections removes the low-frequency carrier from the measurement.
struct Measurements {double normal1, normal2, loud, normalAfterLoud;};
Measurements runEngine(int mode, double level=1.0) {
 DeEsserEngine e;
 e.prepare(sr);
 DeEsserEngine::Settings s;
 s.mode=mode;s.threshold=(mode==2?-24.f:-47.f);s.ratio=14.f;s.range=18.f;
 s.attack=1.0f;s.release=45.f;s.phraseSensitivity=5.f;
 const int total=48000;
 std::vector<double> accumulated(4,0.0);
 std::vector<int> counts(4,0);
 int begin[4]={4800,12480,20160,29760};
 int end[4]={begin[0]+3840,begin[1]+3840,begin[2]+3840,begin[3]+3840};
 double amplitudes[4]={.045,.045,.20,.045};
 constexpr int blockSize=127;
 for(int offset=0;offset<total;offset+=blockSize) {
  int n=std::min(blockSize,total-offset);
  juce::AudioBuffer<float> audio(1,n);
  for(int i=0;i<n;++i) {
   const int t=offset+i;
   double hf=0.0;
   for(int k=0;k<4;++k) if(t>=begin[k] && t<end[k]) hf=amplitudes[k];
   const double in=level*(.18*std::sin(2*pi*190*t/sr)+hf*std::sin(2*pi*5860*t/sr));
   audio.setSample(0,i,(float)in);
  }
  e.process(audio,s);
  for(int i=0;i<n;++i) {
   const int t=offset+i;
   for(int k=0;k<4;++k) if(t>=begin[k]+1200 && t<end[k]-300) {
    accumulated[k] += audio.getSample(0,i)*std::sin(2*pi*5860*t/sr);
    ++counts[k];
   }
  }
 }
 Measurements m{};
 double* fields[4]={&m.normal1,&m.normal2,&m.loud,&m.normalAfterLoud};
 for(int k=0;k<4;++k) *fields[k]=2.0*accumulated[k]/counts[k];
 return m;
}
}

int main() {
 PhraseAwareDetector tracker;
 tracker.prepare(sr);
 // One utterance with ordinary sibilants and one unusually prominent one.
 push(tracker,.2,.0001,100);
 const auto first=push(tracker,.2,.03,90);
 assert(!first.everEligible && !first.everReady); // learn first consonant
 push(tracker,.2,.0001,100);
 const auto normal=push(tracker,.2,.03,90);
 assert(normal.everEligible && normal.everReady);
 const double firstBaseline=normal.baselineAtEnd;
 push(tracker,.2,.0001,100);
 const auto loud=push(tracker,.2,.18,90);
 assert(loud.everEligible);
 // Over-loud events must not drag the baseline upward.
 push(tracker,.2,.0001,120);
 const auto afterLoud=push(tracker,.2,.03,90);
 assert(std::abs(afterLoud.baselineAtEnd-firstBaseline)<2.0);
 assert(afterLoud.learnedAtEnd>=2);
 // New phrase after a real gap starts by learning afresh.
 push(tracker,0.0,0.0,700);
 const auto afterGap=push(tracker,.2,.03,80);
 assert(!afterGap.everEligible && !afterGap.everReady);

 // The engine must leave the first/normal sibilants relatively natural while
 // attenuating the third event; fixed Split should reduce the normal ones.
 const auto adaptive=runEngine(2), fixed=runEngine(0), quiet=runEngine(2,.2);
 std::printf("Phrase Aware amplitudes normal1 %.5f normal2 %.5f loud %.5f next %.5f\n",
             adaptive.normal1,adaptive.normal2,adaptive.loud,adaptive.normalAfterLoud);
 std::printf("Fixed Split   amplitudes normal1 %.5f normal2 %.5f loud %.5f next %.5f\n",
             fixed.normal1,fixed.normal2,fixed.loud,fixed.normalAfterLoud);
 assert(adaptive.normal1>.031);
 assert(adaptive.normal2>.027);
 assert(adaptive.loud<.17);
 assert(adaptive.normal2>fixed.normal2*1.20);
 assert(adaptive.normalAfterLoud>.025);
 // Phrase detection should still work on a much quieter recording with the
 // default -24 dB Threshold setting: baseline-relative, not absolute gating.
 std::printf("Quiet recording: normal %.5f, loud %.5f (raw .009 / .040)\n",
             quiet.normal2,quiet.loud);
 assert(quiet.normal2>.006);
 assert(quiet.loud<.034);
 puts("phrase-aware regression passed");
}
