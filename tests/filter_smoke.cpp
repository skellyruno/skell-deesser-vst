#include "../Source/dsp/SidechainFilter.h"
#include "../Source/dsp/EnvelopeFollower.h"
#include <cmath>
#include <cstdio>
int main(){
 SidechainFilter f;f.prepare(48000);
 auto rms=[&](double frequency){f.reset(); double e=0; const int n=24000;for(int i=0;i<n;++i){auto v=f.process(std::sin(2*3.141592653589793*frequency*i/48000));if(i>=4000)e+=v*v;}return std::sqrt(e/20000.);};
 auto low=rms(150),target=rms(5860),top=rms(14000);
 std::printf("detector RMS 150Hz %.6f / 5860Hz %.6f / 14kHz %.6f\n",low,target,top);
 if(!(target>low*10 && target>top*1.2))return 1;
 EnvelopeFollower env;env.prepare(48000);double result=0;
 for(int i=0;i<48000;++i)result=env.process(.5);
 if(std::abs(result-.5)>1e-5)return 2;
 for(int i=0;i<48000;++i)result=env.process(0);
 if(result>.02)return 3;
 return 0;
}
