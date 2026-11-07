#include "../Source/dsp/DeEsserEngine.h"
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <new>
// Track heap allocations only while isolated DSP process() executes.
static bool track=false;
static std::atomic<int> allocations{0};
void* operator new(std::size_t n){
 if(track)++allocations;
 if(void* p=std::malloc(n?n:1))return p;
 throw std::bad_alloc();
}
void* operator new[](std::size_t n){
 if(track)++allocations;
 if(void* p=std::malloc(n?n:1))return p;
 throw std::bad_alloc();
}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}
int main(){
 DeEsserEngine e;e.prepare(48000.);
 juce::AudioBuffer<float> b(2,513);
 DeEsserEngine::Settings s;
 for(int j=0;j<700;++j){
  s.mode=j%3;s.frequency=(j&1)?19800.f:1100.f;
  s.filterType=j%5;s.hpf=j%3==0;s.monitor=j%7==0;
  s.ratio=j%2?20.f:1.f;s.range=j%2?24.f:0.f;
  s.link=j%2?100.f:0.f;
  for(int i=0;i<513;++i){
   b.setSample(0,i,(float)(.3*std::sin((i+j)*.19)));
   b.setSample(1,i,(float)(.2*std::cos((i+j)*.31)));
  }
  track=true;
  e.process(b,s);
  track=false;
 }
 assert(allocations.load()==0 && "Audio callback must not allocate");
 std::printf("REALTIME_ALLOC_PASS: %d allocations during 700 hostile DSP blocks\n",allocations.load());
}
