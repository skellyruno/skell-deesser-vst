#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

// A de-esser MUST be strictly dry whenever the attenuation curve is at unity,
// including on the very first sample and during host cutoff/mode automation.
int main() {
 constexpr double pi = 3.14159265358979323846;
 for(double rate:{8000.,44100.,48000.,96000.,192000.}) {
  for(int mode=0;mode<3;++mode) {
   for(int test=0;test<3;++test) {
    DeEsserEngine engine;
    engine.prepare(rate);
    DeEsserEngine::Settings s;
    s.mode=mode; s.monitor=0; s.output=0;
    s.frequency=5860.f; s.threshold=test==2 ? 0.f : -60.f;
    s.range=test==1 ? 0.f : 24.f;
    s.ratio=test==0 ? 1.f : 20.f;
    double maxError=0.;
    for(int block=0;block<90;++block) {
     const int size=1+(block*199)%1024;
     juce::AudioBuffer<float> buffer(2,size);
     for(int i=0;i<size;++i) {
      const double t=block*1024+i;
      const float x=(float)(.20*std::sin(2*pi*190*t/rate)
                          +.16*std::sin(2*pi*5860*t/rate));
      buffer.setSample(0,i,x);
      buffer.setSample(1,i,-x);
     }
     s.frequency=(block%2)?1000.f:20000.f;
     s.filterType=block%5;
     s.hpf=(block&1)!=0;
     engine.process(buffer,s);
     for(int i=0;i<size;++i) {
      const double t=block*1024+i;
      const float x=(float)(.20*std::sin(2*pi*190*t/rate)
                          +.16*std::sin(2*pi*5860*t/rate));
      maxError=std::max(maxError,std::abs((double)buffer.getSample(0,i)-x));
      maxError=std::max(maxError,std::abs((double)buffer.getSample(1,i)+x));
      assert(std::isfinite(buffer.getSample(0,i))
          && std::isfinite(buffer.getSample(1,i)));
     }
    }
    if(maxError>1.e-6) {
     std::fprintf(stderr,"Dry path drift: rate %.0f mode %d test %d error %.8g\n",
                  rate,mode,test,maxError);
     return 1;
    }
   }
  }
 }
 std::puts("TRANSPARENT_IDLE_PASS: no-reduction path remains exact dry during automation");
}
