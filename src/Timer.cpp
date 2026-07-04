#include "chip8/Timer.hpp"

namespace chip8 {

void Timer::Reset() {
    delayTimer_ = 0;
    soundTimer_ = 0;
}

void Timer::Tick() {
    if (delayTimer_ > 0) {
        --delayTimer_;
    }
    if (soundTimer_ > 0) {
        --soundTimer_;
    }
}

}  // namespace chip8
