#pragma once

#include <string>

#include "chip8/CPU.hpp"
#include "chip8/Display.hpp"
#include "chip8/Keyboard.hpp"
#include "chip8/Memory.hpp"
#include "chip8/Timer.hpp"

namespace chip8 {

/// Top-level facade that owns and wires together every emulator subsystem
/// (Memory, Display, Keyboard, Timer, CPU). Application drives this class;
/// this class knows nothing about raylib or windowing.
class Chip8 {
public:
    Chip8();

    /// Resets every subsystem to its power-on state.
    void Reset();

    /// Resets the machine and loads the given ROM file at 0x200.
    void LoadROM(const std::string& path);

    /// Executes a single CPU instruction.
    void RunCycle();

    /// Advances the delay/sound timers by one tick. Should be called at
    /// a steady 60Hz.
    void UpdateTimers();

    [[nodiscard]] Memory& GetMemory() noexcept { return memory_; }
    [[nodiscard]] const Memory& GetMemory() const noexcept { return memory_; }

    [[nodiscard]] Display& GetDisplay() noexcept { return display_; }
    [[nodiscard]] const Display& GetDisplay() const noexcept { return display_; }

    [[nodiscard]] Keyboard& GetKeyboard() noexcept { return keyboard_; }
    [[nodiscard]] const Keyboard& GetKeyboard() const noexcept { return keyboard_; }

    [[nodiscard]] Timer& GetTimer() noexcept { return timer_; }
    [[nodiscard]] const Timer& GetTimer() const noexcept { return timer_; }

    [[nodiscard]] CPU& GetCPU() noexcept { return cpu_; }
    [[nodiscard]] const CPU& GetCPU() const noexcept { return cpu_; }

private:
    Memory memory_;
    Display display_;
    Keyboard keyboard_;
    Timer timer_;
    CPU cpu_;
};

}  // namespace chip8
