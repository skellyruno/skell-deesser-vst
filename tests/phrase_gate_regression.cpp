#include "../Source/dsp/PhraseAwareDetector.h"
#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

static void processEvent(PhraseAwareDetector& d, double voice, double sib,
                         int count, int& maxEvents, bool& eligible) {
 for (int i = 0; i < count; ++i) {
  const auto state=d.process(voice,sib,-24.,5.);
  maxEvents=std::max(maxEvents,state.learnedEvents);
  eligible |= state.eligible;
 }
}

int main() {
 constexpr double sr=48000.0;
 PhraseAwareDetector d;
 d.prepare(sr);
 int events=0;
 bool eligible=false;
 // A single wavering consonant should remain ONE learning event despite
 // repeatedly dipping close to the initial-event threshold.
 processEvent(d,0.20,0.003,4800,events,eligible); // broadband vowel, no event
 // Five 15 ms / 45 ms pulses: the original sharp threshold split these
 // into multiple events, while the release threshold keeps one event open.
 for (int j=0;j<5;++j) {
  processEvent(d,0.20,0.014,720,events,eligible);
  processEvent(d,0.20,0.006,2160,events,eligible);
 }
 processEvent(d,0.20,0.001,4800,events,eligible); // complete event
 assert(events==1 && !eligible);
 // New event has enough context to be eligible, and a genuine prolonged
 // breathless pause resets the learned baseline.
 processEvent(d,0.20,0.050,4800,events,eligible);
 assert(eligible);
 processEvent(d,0.,0.,48000,events,eligible);
 auto cleared=d.process(.2,.001,-24.,5.);
 assert(!cleared.ready && cleared.learnedEvents==0);

 // Pathological simultaneous automation must be finite and bounded. This
 // exercises the same independent mode parameters used by JUCE hosts.
 DeEsserEngine e;
 e.prepare(sr);
 DeEsserEngine::Settings s;
 s.frequency=5860.f;s.threshold=-38.f;s.ratio=13.f;s.range=18.f;
 s.attack=.4f;s.release=22.f;
 for(int block=0;block<400;++block) {
  s.mode=block%3; s.monitor=block%17==0;
  s.frequency=(block%2)?19000.f:1000.f;
  juce::AudioBuffer<float> b(2,73);
  for(int i=0;i<73;++i) {
   b.setSample(0,i,(float)(.34*std::sin((block*73+i)*.2)));
   b.setSample(1,i,(float)(.26*std::cos((block*73+i)*.33)));
  }
  e.process(b,s);
  for(int c=0;c<2;++c)for(int i=0;i<73;++i) {
   const float x=b.getSample(c,i);
   assert(std::isfinite(x));
   assert(std::abs(x)<3.f); // no exploding filter or negative mix weight
  }
 }
 std::puts("phrase gate and fast mode automation tests passed");
}
