#pragma once
#include <algorithm>
#include <cmath>
class EnvelopeFollower {
public:
 void prepare(double rate) { sampleRate=std::max(1.,rate); value=0.; update(2.,85.); }
 void update(double attackMs,double releaseMs) {
  a=std::exp(-1.0/(sampleRate*std::max(.1,attackMs)*.001));
  r=std::exp(-1.0/(sampleRate*std::max(1.,releaseMs)*.001));
 }
 double process(double x) { x=std::abs(x);const auto c=x>value?a:r;value=c*value+(1-c)*x;return value; }
private: double sampleRate=48000.,a=.9,r=.99,value=0.;
};
