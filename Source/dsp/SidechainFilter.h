#pragma once
#include <algorithm>
#include <array>
#include <cmath>

// Transposed direct-form II, double precision. State stays alive when modes change.
class Biquad {
public:
 enum Type { lowpass, highpass, bandpass };
 void reset() noexcept { z1 = z2 = 0.0; }
 void set(Type type, double hz, double q, double sr) noexcept {
  sr = std::max(1000.0, sr);
  hz = std::clamp(hz, 10.0, sr * .44); q = std::max(.1, q);
  constexpr double pi = 3.14159265358979323846;
  const double w = 2.0 * pi * hz / sr;
  const double c = std::cos(w), s = std::sin(w);
  const double alpha = s / (2.0 * q), norm = 1.0 / (1.0 + alpha);
  if (type == lowpass) { b0 = (1-c)*.5*norm; b1 = (1-c)*norm; b2 = b0; }
  else if (type == highpass) { b0 = (1+c)*.5*norm; b1 = -(1+c)*norm; b2 = b0; }
  else { b0 = alpha*norm; b1 = 0; b2 = -alpha*norm; }
  a1 = -2*c*norm; a2 = (1-alpha)*norm;
 }
 double process(double x) noexcept {
  const double y = b0*x + z1;
  z1 = b1*x - a1*y + z2;
  z2 = b2*x - a2*y;
  return y;
 }
private:
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
};

class SidechainFilter {
public:
 void prepare(double sr) noexcept {
  sampleRate = std::max(1000.0,sr);
  reset(); configure(5860.0, 2, true);
 }
 void reset() noexcept {
  for (auto& f : filters) f.reset();
  weights = { 0.0, 0.0, 1.0, 0.0 };
  target = weights;
 }
 void configure(double freq, int mode, bool hpf) noexcept {
  // All filter stages run in every mode, including when bypassed. This avoids
  // stale IIR state and provides click-resistant changes between tap outputs.
  const auto f = std::clamp(freq, 70.0, sampleRate*.32);
  filters[0].set(Biquad::bandpass, f, 1.25, sampleRate);
  filters[1].set(Biquad::highpass, std::min(1800.0, f*.42), .70710678, sampleRate);
  filters[2].set(Biquad::highpass, std::min(2400.0, f*.58), .70710678, sampleRate);
  filters[3].set(Biquad::bandpass, f, 1.25, sampleRate);
  const int tap = mode == 3 || mode == 4 ? 3 : mode == 2 ? 2 : (mode == 1 || hpf) ? 1 : 0;
  target.fill(0.0);
  target[static_cast<size_t>(tap)] = 1.0;
 }
 // A restored session should start on the selected detector tap, not crossfade
 // from the factory HPF-12 tap during the first few milliseconds of playback.
 // This is only used on initialisation; subsequent mode changes still ramp.
 void configureImmediate(double freq, int mode, bool hpf) noexcept {
  configure(freq,mode,hpf);
  weights=target;
 }
 void setModeSmoothing(double ms) noexcept {
  const auto seconds = std::max(.001, ms * .001);
  smooth = 1.0 - std::exp(-1.0/(sampleRate*seconds));
 }
 double process(double input) noexcept {
  const double bp = filters[0].process(input);
  const double hp6 = filters[1].process(bp);
  const double hp12 = filters[2].process(hp6);
  const double narrow = filters[3].process(hp12);
  const double taps[4] {bp,hp6,hp12,narrow};
  double result = 0.0;
  for (size_t i=0;i<4;++i) {
   weights[i] += (target[i] - weights[i]) * smooth;
   result += taps[i] * weights[i];
  }
  return result;
 }
private:
 double sampleRate = 48000.0, smooth = .004;
 std::array<Biquad,4> filters;
 std::array<double,4> weights{{0,0,1,0}},target{{0,0,1,0}};
};
