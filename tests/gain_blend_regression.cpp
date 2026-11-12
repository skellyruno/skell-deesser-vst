#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

// The optimized gain path must be equivalent across all fully-selected modes
// AND during simultaneous Split/Wide/Phrase automation crossfades.
int main() {
 const std::array<double,7> reductions{0.0,0.001,0.5,3.0,9.0,16.0,24.0};
 const std::array<double,8> blends{0.0,1e-8,0.01,0.2,0.5,0.8,1.0-1e-8,1.0};
 const std::array<double,5> bands{-1.0,-0.25,0.0,0.28,0.9};
 int iterations=0;
 for(const auto reduction:reductions) for(const auto phraseReduction:reductions)
 for(const auto phraseBlend:blends) for(const auto wideBlend:blends)
 for(const auto input:bands) for(const auto sibilance:bands) {
  const auto gain=std::pow(10.0,-reduction/20.0);
  const auto phraseGain=std::pow(10.0,-phraseReduction/20.0);
  const auto split=input+(gain-1.0)*sibilance;
  const auto wide=input*gain;
  const auto phraseSplit=input+(phraseGain-1.0)*sibilance;
  const auto original=(split+(wide-split)*wideBlend)*(1.0-phraseBlend)+phraseSplit*phraseBlend;
  const auto optimizedGain=phraseBlend<1.0&&reduction>0.0?gain:1.0;
  const auto optimizedPhraseGain=phraseBlend>0.0&&phraseReduction>0.0?phraseGain:1.0;
  const auto optimizedSplit=input+(optimizedGain-1.0)*sibilance;
  const auto optimizedWide=input*optimizedGain;
  const auto optimizedPhraseSplit=input+(optimizedPhraseGain-1.0)*sibilance;
  const auto candidate=(optimizedSplit+(optimizedWide-optimizedSplit)*wideBlend)
                         *(1.0-phraseBlend)+optimizedPhraseSplit*phraseBlend;
  if(!std::isfinite(candidate) || std::abs(original-candidate)>5.0e-13) {
   std::fprintf(stderr,"Gain blend mismatch reductions %.3f/%.3f blend %.10f diff %.15g\\n",
                reduction,phraseReduction,phraseBlend,original-candidate);
   return 1;
  }
  ++iterations;
 }
 std::printf("Gain blend regression: %d identical combinations\\n",iterations);
}