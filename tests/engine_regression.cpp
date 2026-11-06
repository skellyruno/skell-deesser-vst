#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>
constexpr double pi=3.14159265358979323846;
static double project(DeEsserEngine& e,int mode,double hz) {
 const double sr=48000.0;
 e.prepare(sr);
 DeEsserEngine::Settings s;
 s.mode=mode;s.attack=.1f;s.release=35.f;s.threshold=-48.f;
 s.ratio=12.f;s.range=18.f;
 const int block=127,total=48000;
 double sinp=0,cosp=0;
 for(int offset=0;offset<total;offset+=block) {
  int count=std::min(block,total-offset);
  juce::AudioBuffer<float> audio(1,count);
  for(int i=0;i<count;++i) {
   int t=i+offset;
   audio.setSample(0,i,(float)(.25*std::sin(2*pi*150*t/sr)+.3*std::sin(2*pi*5860*t/sr)));
  }
  e.process(audio,s);
  for(int i=0;i<count;++i) if(offset+i>=12000) {
   const int t=i+offset;
   sinp+=audio.getSample(0,i)*std::sin(2*pi*hz*t/sr);
   cosp+=audio.getSample(0,i)*std::cos(2*pi*hz*t/sr);
  }
 }
 return 2.*std::hypot(sinp,cosp)/36000.;
}
int main() {
 DeEsserEngine engine;
 const double splitBass=project(engine,0,150),splitS=project(engine,0,5860);
 const double wideBass=project(engine,1,150),wideS=project(engine,1,5860);
 std::printf("split amplitudes 150 %.5f / 5860 %.5f ; wide 150 %.5f / 5860 %.5f\n",splitBass,splitS,wideBass,wideS);
 assert(splitBass>.21 && splitBass<.29);
 assert(splitS<.27 && splitS>.0);
 assert(wideBass<splitBass*.92 && wideS<splitS*.99);
 // Automation stress: fractional blocks, rapid mode/filter changes, unusual rates.
 for(double sr : {8000.,22050.,44100.,48000.,96000.,192000.}) {
  engine.prepare(sr);
  DeEsserEngine::Settings s;
  s.threshold=-60.f;s.ratio=20.f;s.range=24.f;s.attack=.1f;s.release=10;
  auto started=std::chrono::steady_clock::now();
  for(int chunk=0;chunk<220;++chunk) {
   const int n=1+(chunk*211)%1024;
   juce::AudioBuffer<float> audio(chunk%3?2:1,n);
   s.frequency=1000.f+19000.f*((chunk*17)%101)/100.f;
   s.ratio=1.f+19.f*((chunk*7)%101)/100.f;
   s.range=24.f*((chunk*19)%101)/100.f;
   s.filterType=chunk%5;s.hpf=(chunk%2)==0;
   s.mode=chunk%3;s.phraseSensitivity=1.f+11.f*((chunk*13)%101)/100.f;
   s.monitor=(chunk%5)==4;
   s.link=(float)(chunk%101);s.output=(float)((chunk%25)-12);
   for(int c=0;c<audio.getNumChannels();++c)
    for(int i=0;i<n;++i)
     audio.setSample(c,i,(float)(.6*std::sin((chunk*n+i)*(c?0.37:0.11))));
   engine.process(audio,s);
   for(int c=0;c<audio.getNumChannels();++c)for(int i=0;i<n;++i) {
    assert(std::isfinite(audio.getSample(c,i)));
    assert(std::abs(audio.getSample(c,i))<8.f);
   }
  }
  auto duration=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  std::printf("automation stress %.0f Hz: %.2f ms (informational)\n",sr,duration);
 }
 // Out-of-phase stereo must still show a waveform (mid cancellation is wrong).
 engine.prepare(48000);
 juce::AudioBuffer<float> phase(2,1024);
 for(int i=0;i<1024;++i) {
  float x=(float)(.7*std::sin(i*.27)); phase.setSample(0,i,x);phase.setSample(1,i,-x);
 }
 engine.process(phase,DeEsserEngine::Settings{});
 std::array<float,WaveformHistory::length> values{};
 engine.waveform.copy(values.data(),values.size());
 assert(*std::max_element(values.begin(),values.end())>.5f);
 puts("engine regression checks passed");
}
