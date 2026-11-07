#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
// Deterministic hostile-host regression. Synthetic DSP, not perceptual proof.
static uint64_t seed=0x6a09e667f3bcc909ULL;
static uint32_t rnd() noexcept {
 seed^=seed<<13; seed^=seed>>7; seed^=seed<<17;
 return static_cast<uint32_t>(seed>>16);
}
static float synth(int sample,int channel,int scenario) {
 constexpr float pi=3.14159265358979323846f;
 switch(scenario%10){
 case 0:return 0.f;
 case 1:return sample==0?1.f:0.f;
 case 2:return -.32f;
 case 3:return (sample&1)?1.f:-1.f;
 case 4:return std::numeric_limits<float>::denorm_min();
 case 5:return sample==7?std::numeric_limits<float>::infinity():.13f;
 case 6:return sample==11?std::numeric_limits<float>::quiet_NaN():-.18f;
 case 7:return sample==15?-1.e25f:1.e25f;
 case 8:return channel?-.45f*std::sin(2*pi*.057f*sample):.45f*std::sin(2*pi*.057f*sample);
 default:return static_cast<float>((int)(rnd()%20001)-10000)/10000.f;
 }
}
static DeEsserEngine::Settings hostile(int iter){
 DeEsserEngine::Settings s;
 const float nan=std::numeric_limits<float>::quiet_NaN();
 const float inf=std::numeric_limits<float>::infinity();
 s.mode=iter%9-3;s.monitor=iter%7-2;s.filterType=iter%15-4;
 s.hpf=iter%2==0;
 s.frequency=iter%13==0?nan:iter%13==1?inf:iter%13==2?-1000.f:iter%13==3?999999.f:float(1000+(rnd()%19001));
 s.threshold=iter%11==0?nan:iter%11==1?-inf:iter%11==2?9999.f:-float(rnd()%600)/10.f;
 s.ratio=iter%17==0?nan:iter%17==1?inf:iter%17==2?-5.f:1.f+float(rnd()%190)/10.f;
 s.range=iter%19==0?nan:iter%19==1?inf:iter%19==2?-9999.f:float(rnd()%250)/10.f;
 s.attack=iter%23==0?nan:iter%23==1?0.f:float(rnd()%300)/10.f;
 s.release=iter%29==0?nan:iter%29==1?inf:float(10+rnd()%491);
 s.link=iter%31==0?nan:iter%31==1?inf:float(rnd()%101);
 s.output=iter%37==0?nan:iter%37==1?-inf:float((int)(rnd()%250)-125)/10.f;
 s.phraseSensitivity=iter%41==0?nan:iter%41==1?inf:float(1+rnd()%120)/10.f;
 return s;
}
int main(){
 using Clock=std::chrono::steady_clock;
 const auto start=Clock::now();
 long long totalSamples=0,blocks=0;
 for(double rate:{8000.,11025.,22050.,44100.,48000.,88200.,96000.,176400.,192000.,384000.,768000.}){
  DeEsserEngine e;e.prepare(rate);
  constexpr int sizes[]{0,1,2,3,7,16,31,63,64,127,128,255,511,512,1023,2048,8192};
  for(int b=0;b<350;++b){
   const int size=sizes[b%17],channels=1+(b%2);
   juce::AudioBuffer<float> audio(channels,size);
   const int scenario=b%10;
   for(int c=0;c<channels;++c)
    for(int i=0;i<size;++i)audio.setSample(c,i,synth(i+b,c,scenario));
   auto s=hostile(b);
   e.process(audio,s);
   for(int c=0;c<channels;++c)for(int i=0;i<size;++i){
    const auto y=audio.getSample(c,i);
    assert(std::isfinite(y));
    assert(std::abs(y)<=1000001.f);
   }
   assert(std::isfinite(e.sidechainLevelDb.load()));
   assert(std::isfinite(e.gainReductionDb.load()));
   assert(std::isfinite(e.phraseBaselineDb.load()));
   assert(e.gainReductionDb.load()>=0.f && e.gainReductionDb.load()<=24.01f);
   ++blocks;totalSamples+=size;
   if(b==174)e.prepare(rate);
  }
  // Range=0 must be transparent from the FIRST frame of a restored preset.
  for(int mode=0;mode<3;++mode){
   e.prepare(rate);
   DeEsserEngine::Settings s;
   s.mode=mode;s.range=0;s.threshold=-60;s.ratio=20;
   juce::AudioBuffer<float> audio(2,64);
   std::array<float,128> input{};
   for(int i=0;i<64;++i){
    input[i]=(float)(.6*std::sin(i*.2));
    input[i+64]=-input[i];
    audio.setSample(0,i,input[i]);audio.setSample(1,i,input[64+i]);
   }
   e.process(audio,s);
   for(int i=0;i<64;++i)for(int c=0;c<2;++c)
    assert(std::abs(audio.getSample(c,i)-input[i+c*64])<2.e-6f);
  }
  std::printf("stress %.0f Hz: passed 350 hostile blocks + unity tests\n",rate);
 }
 const auto ms=std::chrono::duration<double,std::milli>(Clock::now()-start).count();
 std::printf("EXTREME_STRESS_PASS: %lld buffers, %lld samples, %.1f ms (informational; hardware dependent)\n",blocks,totalSamples,ms);
}
