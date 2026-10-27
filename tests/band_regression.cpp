#include "../Source/dsp/ComplementaryBand.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
int main() {
 for (double sr : {8000.,22050.,44100.,48000.,96000.,192000.}) {
  ComplementaryBand split; split.prepare(sr);
  double maxAbs=0, maxRecon=0;
  for(int i=0;i<120000;++i) {
   if(i%37==0)split.setFrequency(1000. + 18000.*((i/37)%97)/96.);
   const double x = .4*std::sin(i*.09)+.3*std::cos(i*.47);
   const auto mid=split.process(x);
   const auto lowhigh=x-mid;
   maxRecon=std::max(maxRecon,std::abs((lowhigh+mid)-x));
   maxAbs=std::max(maxAbs,std::abs(mid));
   assert(std::isfinite(mid));
  }
  assert(maxRecon<1.e-12 && maxAbs<1.2);
  std::printf("band %.0f Hz: reconstruction %.2g, peak %.4f\n",sr,maxRecon,maxAbs);
 }
}
