#pragma once
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>
// Single audio-thread writer, single GUI-thread reader. Per-cell atomic publication.
// The UI may observe one cell from the previous wrap, but never a data race.
class WaveformHistory {
public:
 static constexpr unsigned length = 256;
 void prepare(double sampleRate) noexcept {
  stride = std::max(1u, static_cast<unsigned>(sampleRate / 180.0));
  count = 0; maximum = 0.f;
  for (auto& value : peaks) value.store(0.f, std::memory_order_relaxed);
  writePosition.store(0, std::memory_order_relaxed);
 }
 void push(float sample) noexcept {
  if (!std::isfinite(sample)) sample = 0.f;
  maximum = std::max(maximum, std::min(1.f, std::abs(sample)));
  if (++count >= stride) {
   auto next = writePosition.load(std::memory_order_relaxed) + 1;
   peaks[next % length].store(maximum, std::memory_order_relaxed);
   writePosition.store(next, std::memory_order_release);
   maximum = 0.f; count = 0;
  }
 }
 void copy(float* destination, unsigned size) const noexcept {
  const auto end = writePosition.load(std::memory_order_acquire);
  const auto n = std::min(size, length);
  for (unsigned i = 0; i < n; ++i) {
   auto age = n - 1 - i;
   destination[i] = end > age ? peaks[(end - age) % length].load(std::memory_order_relaxed) : 0.f;
  }
 }
private:
 std::array<std::atomic<float>, length> peaks{};
 std::atomic<uint64_t> writePosition{0};
 unsigned stride = 256, count = 0;
 float maximum = 0.f;
};
