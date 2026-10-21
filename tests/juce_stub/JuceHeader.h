#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
namespace juce {
 template<class T> T jlimit(T lo,T hi,T x){return std::clamp(x,lo,hi);}
 namespace ValueSmoothingTypes{struct Linear{};}
 template<class T,class Mode> class SmoothedValue {
  T current=0,target=0,step=0;int remaining=0;double sr=48000, seconds=.01;
 public:
  void reset(double rate,double time){sr=rate;seconds=time;remaining=0;}
  void setCurrentAndTargetValue(T x){current=target=x;remaining=0;}
  void setTargetValue(T x){if(target==x)return;target=x;remaining=std::max(1,int(sr*seconds));step=(target-current)/remaining;}
  T getNextValue(){if(remaining>0){current+=step;--remaining;if(remaining==0)current=target;}return current;}
 };
 struct Decibels{static double decibelsToGain(double x){return std::pow(10.,x/20.);}};
 template<class T>class AudioBuffer{
  std::vector<std::vector<T>> data;
 public:
  AudioBuffer(int channels,int samples):data(channels,std::vector<T>(samples)){}
  int getNumChannels()const{return (int)data.size();}
  int getNumSamples()const{return data.empty()?0:(int)data[0].size();}
  T getSample(int c,int i)const{return data[c][i];}
  void setSample(int c,int i,T x){data[c][i]=x;}
 };
}
