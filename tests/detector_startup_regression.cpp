#include "../Source/dsp/DeEsserEngine.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
 constexpr double pi=3.14159265358979323846;
 for(double sr:{8000.0,44100.0,48000.0,96000.0,192000.0}) {
  for(int filterType=0;filterType<=4;++filterType) {
   for(bool hpf:{false,true}) {
    DeEsserEngine e; e.prepare(sr);
    SidechainFilter reference;
    reference.prepare(sr);
    reference.setModeSmoothing(6.0);
    reference.configureImmediate(std::min(4200.0,sr*.30),filterType,hpf);
    DeEsserEngine::Settings s;
    s.mode=0; s.monitor=1; s.range=0; s.output=0;
    s.frequency=4200; s.filterType=filterType; s.hpf=hpf;
    juce::AudioBuffer<float> buffer(1,192);
    for(int i=0;i<192;++i) {
     const float sample=static_cast<float>(.47*std::sin(2*pi*1800*i/sr)
                      + .24*std::sin(2*pi*4200*i/sr));
     buffer.setSample(0,i,sample);
    }
    std::array<float,192> expected{};
    for(int i=0;i<192;++i)expected[static_cast<size_t>(i)] =
          static_cast<float>(reference.process(buffer.getSample(0,i)));
    e.process(buffer,s);
    for(int i=0;i<192;++i) {
     const float actual=buffer.getSample(0,i);
     assert(std::isfinite(actual));
     if(std::abs(actual-expected[static_cast<size_t>(i)])>0.000005f) {
      std::fprintf(stderr,"startup detector mismatch: rate %.0f type %d hpf %d sample %d expected %g actual %g\n",
                   sr,filterType,int(hpf),i,expected[static_cast<size_t>(i)],actual);
      return 1;
     }
    }
   }
  }
 }
 SidechainFilter smoothed, immediate;
 smoothed.prepare(48000.0); immediate.prepare(48000.0);
 smoothed.setModeSmoothing(6.0); immediate.setModeSmoothing(6.0);
 smoothed.configure(4200.0,3,false);
 immediate.configureImmediate(4200.0,3,false);
 double difference=0;
 for(int i=0;i<96;++i) {
  const auto sample=.5*std::sin(i*.29);
  difference+=std::abs(smoothed.process(sample)-immediate.process(sample));
 }
 assert(difference>0.00001);
 std::puts("detector first-block preset monitoring and later smooth transitions passed");
}
