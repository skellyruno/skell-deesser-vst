#pragma once
#include <algorithm>
#include <cmath>
class Biquad {
public:
 enum Type { lowpass, highpass, bandpass };
 void reset() noexcept { z1=z2=0.0; }
 void set(Type type,double hz,double q,double sr) noexcept {
  hz=std::clamp(hz,10.0,std::max(20.0,sr*.46)); q=std::max(.1,q);
  const double w=2.0*3.14159265358979323846*hz/sr,c=std::cos(w),s=std::sin(w),alpha=s/(2.0*q),norm=1.0/(1.0+alpha);
  if(type==lowpass) { b0=(1-c)*.5*norm;b1=(1-c)*norm;b2=b0; }
  else if(type==highpass) { b0=(1+c)*.5*norm;b1=-(1+c)*norm;b2=b0; }
  else { b0=alpha*norm;b1=0;b2=-alpha*norm; }
  a1=-2*c*norm;a2=(1-alpha)*norm;
 }
 double process(double x) noexcept { const double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y; }
private:
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
};
class SidechainFilter {
public:
 void prepare(double sr) noexcept { sampleRate=std::max(1000.0,sr);reset();configure(5860,2,true); }
 void reset() noexcept { for(auto& b:filters)b.reset(); }
 void configure(double freq,int mode,bool hpf) noexcept {
  // HPF is a prefilter; detector always selects the sibilance region.
  // Off/HPF 6/HPF 12/Band-Pass/Listen describe additional shaping.
  const auto f=std::clamp(freq,100.0,sampleRate*.43);
  filters[0].set(Biquad::highpass,std::min(1800.0,f*.42),.70710678,sampleRate);
  filters[1].set(Biquad::highpass,std::min(2400.0,f*.58),.70710678,sampleRate);
  filters[2].set(Biquad::bandpass,f,1.25,sampleRate);
  filters[3].set(Biquad::bandpass,f,1.25,sampleRate);
  filterMode=mode;highpassEnabled=hpf;
 }
 double process(double x) noexcept {
  if(highpassEnabled || filterMode==1 || filterMode==2 || filterMode==3 || filterMode==4)
   x=filters[0].process(x);
  if(filterMode==2 || filterMode==3 || filterMode==4)x=filters[1].process(x);
  // In all normal processing modes we use a tuned detector, not broad low-frequency energy.
  x=filters[2].process(x);
  // Band-pass and Listen use a narrower detector response.
  if(filterMode==3 || filterMode==4)x=filters[3].process(x);
  return x;
 }
private:
 double sampleRate=48000;bool highpassEnabled=true;int filterMode=2;Biquad filters[4];
};
