#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
constexpr double pi=3.14159265358979323846;
std::vector<float> render(double sr,int blockSize,int mode,int monitor) {
 constexpr int total=48000;
 DeEsserEngine engine; engine.prepare(sr);
 DeEsserEngine::Settings s;
 s.mode=mode;s.monitor=monitor;s.frequency=5870.f;s.threshold=-32.f;
 s.ratio=9.f;s.range=17.f;s.attack=.7f;s.release=53.f;
 s.link=61.f;s.phraseSensitivity=5.f;s.filterType=3;
 std::vector<float> output(total*2,0.f);
 for(int base=0;base<total;base+=blockSize) {
  const int n=std::min(blockSize,total-base);
  juce::AudioBuffer<float> audio(2,n);
  for(int i=0;i<n;++i) {
   const int t=base+i;
   const auto high=(t%12000<2250)?1.0:0.08;
   const auto low=.26*std::sin(2*pi*195*t/sr);
   const auto sib=high*.33*std::sin(2*pi*5870*t/sr);
   audio.setSample(0,i,(float)(low+sib));
   audio.setSample(1,i,(float)(.75*low-.87*sib));
  }
  engine.process(audio,s);
  for(int i=0;i<n;++i)for(int c=0;c<2;++c) {
   const auto x=audio.getSample(c,i); assert(std::isfinite(x));
   output[(base+i)*2+c]=x;
  }
 }
 return output;
}
}
int main() {
 for(double sr:{44100.,48000.,96000.})for(int mode=0;mode<3;++mode)
  for(int monitor=0;monitor<=1;++monitor) {
   const auto reference=render(sr,1,mode,monitor);
   for(int blockSize:{7,64,127,512,2048,8192}) {
    const auto actual=render(sr,blockSize,mode,monitor);
    double maxError=0.;
    for(size_t i=0;i<actual.size();++i)
     maxError=std::max(maxError,std::abs((double)actual[i]-reference[i]));
    if(maxError>2.e-5){
     std::fprintf(stderr,"partition mismatch sr %.0f mode %d monitor %d block %d error %.8g\n",sr,mode,monitor,blockSize,maxError);
     return 1;
    }
   }
  }
 std::puts("block partition invariant output passed (stereo, three modes, two monitor routes)");
}
