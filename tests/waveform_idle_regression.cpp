#include "../Source/dsp/WaveformHistory.h"
#include <array>
#include <cassert>
#include <cstdio>

int main() {
 WaveformHistory history;
 history.prepare(48000.0);
 const auto start=history.serial();
 for(int i=0;i<256*480;++i)history.push(0.0f);
 assert(history.serial()==start && "silent input must not trigger idle repaints");
 for(int i=0;i<320;++i)history.push(i==0?0.5f:0.0f);
 const auto active=history.serial();
 assert(active>start && "input transient must wake waveform display");
 for(int i=0;i<320;++i)history.push(0.0f);
 assert(history.serial()>active && "nonzero historical peaks must continue scrolling");
 for(int i=0;i<256*480;++i)history.push(0.0f);
 const auto settled=history.serial();
 assert(settled>active && "transient must eventually leave the scrolling ring");
 for(int i=0;i<256*480;++i)history.push(0.0f);
 assert(history.serial()==settled && "once silent, stop repainting");
 std::array<float,WaveformHistory::length> peak{};
 history.copy(peak.data(),static_cast<unsigned>(peak.size()));
 for(const auto value:peak)assert(value==0.0f);
 std::puts("waveform idle repaint suppression and transient wakeup passed");
}
