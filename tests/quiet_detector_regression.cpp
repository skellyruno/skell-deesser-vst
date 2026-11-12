#include "../Source/dsp/DeEsserEngine.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>

// The detector fast path is legal only if it cannot affect either the
// traditional 6 dB soft-knee gain curve or a valid learned phrase threshold.
int main() {
 constexpr double quietFloor=0.0007079457843841379;
 constexpr double phraseEventFloor=0.00158489319246; // -56 dBFS
 const double minimumLearnedDb=20.0*std::log10(phraseEventFloor);
 assert(minimumLearnedDb>-56.00001);
 const double minPhraseThreshold=minimumLearnedDb+0.5;
 assert(minPhraseThreshold>-63.0);
 // Exercise below, at, and just above the gate with every valid threshold.
 for(int ti=0;ti<=240;++ti) {
  const double threshold=-60.0 + ti*.25;
  for(double level: {0.0,1.0e-10,1.0e-8,1.0e-6,
                     quietFloor*.5,quietFloor,quietFloor*1.0000001}) {
   const double exactDb=20.0*std::log10(std::max(level,1.e-8));
   const double optimisedDb=level<=quietFloor?-160.0:exactDb;
   for(double ratio:{1.0,2.0,4.0,20.0})
    for(double range:{0.0,2.0,12.0,24.0}) {
     const double expected=DeEsserEngine::computeReduction(exactDb,threshold,ratio,range);
     const double actual=DeEsserEngine::computeReduction(optimisedDb,threshold,ratio,range);
     if(std::abs(actual-expected)>1.e-12) {
       std::fprintf(stderr,"DSP quiet detector drift at threshold %.3f level %.9g\n",threshold,level);
       return 1;
     }
    }
  }
 }
 // Phrase threshold is never below the quiet gate after any learnable
 // event, even with minimum sensitivity and the strongest threshold bias.
 for(double baseline=-56.0;baseline<=24.0;baseline+=1.0) {
  for(double sensitivity:{1.0,5.0,12.0}) {
   for(double bias:{-6.0,0.0,6.0}) {
    const double relative=baseline+std::max(0.5,sensitivity+bias);
    if(relative<=-63.0)return 2;
   }
  }
 }
 std::puts("quiet detector fast path preserves both soft-knee and phrase gates");
}
