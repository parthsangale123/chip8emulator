#include "chip8/Chip8.hpp"

namespace chip8 {

Chip8::Chip8()
    : memory_(), display_(), keyboard_(), timer_(), cpu_(memory_, display_, keyboard_, timer_) {}

void Chip8::Reset() {
    memory_.Reset();
    display_.Clear();
    timer_.Reset();
    cpu_.Reset();
}

void Chip8::LoadROM(const std::string& path) {
    Reset();
    memory_.LoadROM(path);
}

void Chip8::RunCycle() { cpu_.Cycle(); }

void Chip8::UpdateTimers() { timer_.Tick(); }

}  // namespace chip8
