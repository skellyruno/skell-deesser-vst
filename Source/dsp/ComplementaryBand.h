#pragma once
#include <algorithm>
#include <cmath>

// Two first-order trapezoidal one-pole stages form a complementary three-way split.
// low = LP(lowCut, x); remainder = x-low;
// mid = LP(highCut, remainder); high = remainder-mid.
// low+mid+high == x for every sample, including during cutoff automation.
// Returning the dry sample and subtracting ONLY the mid-band reduction ensures
// sample-accurate unity when gain reduction is zero, without a parallel phase offset.
class ComplementaryBand {
public:
 void prepare(double sampleRate) noexcept {
  rate = std::max(1000.0, sampleRate);
  lowState = highState = 0.0;
  setFrequency(5860.0);
 }
 void reset() noexcept { lowState = highState = 0.0; }
 void setFrequency(double centre) noexcept {
  centre = std::clamp(centre, 30.0, rate * 0.30);
  const double low = std::clamp(centre * 0.53, 20.0, rate * 0.405);
  const double high = std::clamp(centre * 1.85, low * 1.05, rate * 0.445);
  lowA = coefficient(low);
  highA = coefficient(high);
 }
 double process(double input) noexcept {
  const double low = onePole(input, lowState, lowA);
  const double remainder = input - low;
  return onePole(remainder, highState, highA);
 }
private:
 double coefficient(double hz) const noexcept {
  constexpr double pi = 3.14159265358979323846;
  const double g = std::tan(pi * hz / rate);
  return g / (1.0 + g);
 }
 static double onePole(double x, double& state, double a) noexcept {
  const double v = (x - state) * a;
  const double low = state + v;
  state = low + v;
  return low;
 }
 double rate = 48000.0, lowA = .1, highA = .4;
 double lowState = 0.0, highState = 0.0;
};
