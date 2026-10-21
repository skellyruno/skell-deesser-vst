#include "../Source/dsp/DeEsserEngine.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 constexpr int n=4096;
 DeEsserEngine e;e.prepare(48000);
 DeEsserEngine::Settings s;
 s.threshold=0.f; s.range=0.f;
 juce::AudioBuffer<float> audio(2,n);
 for(int i=0;i<n;++i){auto v=.7f*std::sin(i*.12f);audio.setSample(0,i,v);audio.setSample(1,i,v*.8f);}
 auto copy=audio;
 e.process(audio,s);
 float maxDiff=0;
 for(int c=0;c<2;++c)for(int i=0;i<n;++i)maxDiff=std::max(maxDiff,std::abs(audio.getSample(c,i)-copy.getSample(c,i)));
 assert(maxDiff<1.e-6f);
 s.threshold=-60;s.range=24;s.ratio=20;
 juce::AudioBuffer<float> a(2,n);
 for(int i=0;i<n;++i){auto v=.7f*std::sin(2*3.141592653589793*5860*i/48000);a.setSample(0,i,v);a.setSample(1,i,v);}
 e.process(a,s);
 assert(e.gainReductionDb.load()>0.f);
 for(int c=0;c<2;++c)for(int i=0;i<n;++i)assert(std::isfinite(a.getSample(c,i)));
 std::printf("engine unity max error %.9f; reduction %.2f dB\n",maxDiff,e.gainReductionDb.load());
}
