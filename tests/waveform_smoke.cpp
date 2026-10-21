#include "../Source/dsp/WaveformHistory.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main() {
 WaveformHistory h;
 h.prepare(48000);
 float values[WaveformHistory::length]{};
 h.copy(values, WaveformHistory::length);
 for(float v:values) assert(v==0.f);
 for(int i=0;i<48000;++i)h.push((i%100==0)?0.75f:0.05f);
 h.copy(values,WaveformHistory::length);
 float max=0.f;for(auto v:values){assert(std::isfinite(v));max=std::max(max,v);}
 assert(max>=.74f&&max<=.76f);
 h.push(NAN);
 std::printf("waveform max %.3f; ring=%u\n",max,WaveformHistory::length);
}
