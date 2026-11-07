#include "../Source/dsp/DeEsserEngine.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <algorithm>
static constexpr double pi=3.14159265358979323846;
static void block(DeEsserEngine& e, DeEsserEngine::Settings& s, int n, int t, double sr) {
 juce::AudioBuffer<float> b(2,n);
 for(int i=0;i<n;++i) {
  const float signal=(float)(.28*std::sin(2*pi*6200.0*(i+t)/sr));
  b.setSample(0,i,signal);
  b.setSample(1,i,signal*.8f);
 }
 e.process(b,s);
 for(int c=0;c<2;++c)for(int i=0;i<n;++i) {
  const auto sample=b.getSample(c,i);
  assert(std::isfinite(sample) && std::abs(sample)<2.f);
 }
}
int main() {
 for(double sr:{8000.,44100.,48000.,96000.,192000.}) {
  DeEsserEngine e;e.prepare(sr);
  DeEsserEngine::Settings s;
  s.frequency=4000;s.mode=0;s.filterType=2;s.hpf=true;s.threshold=-38;s.ratio=14;s.range=24;
  for(int t=0;t<sr;t+=127)block(e,s,std::min(127,(int)sr-t),t,sr);
  // Allow the initial 5860 -> 4000 Hz startup slew to settle, then measure
  // the steady-state reconfiguration rate independently of that transition.
  const auto settled=e.getFilterReconfigurationCount();
  for(int t=0;t<sr;t+=127)block(e,s,std::min(127,(int)sr-t),t+(int)sr,sr);
  const auto stableCount=e.getFilterReconfigurationCount();
  assert(stableCount-settled<8 && "Static cutoff should not calculate coefficients each 16 samples");
  // Aggressive automation on ratio/range/link, plus fast cutoff sweeps.
  for(int t=0;t<8192;t+=64){
   s.frequency=(t%256)?18000.f:1200.f;
   s.filterType=t%5;
   s.hpf=(t/64)%2;
   s.ratio=(t/64)%2 ? 20.f:1.f;
   s.range=(t/64)%2 ? 24.f:0.f;
   s.link=(t/64)%2 ? 100.f:0.f;
   block(e,s,64,t,sr);
  }
  assert(e.getFilterReconfigurationCount()>stableCount);
  assert(e.getFilterReconfigurationCount() < stableCount+1024);
 }
 puts("filter automation regression passed");
}
