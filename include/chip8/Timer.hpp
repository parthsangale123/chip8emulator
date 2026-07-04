#pragma once

#include <cstdint>

namespace chip8 {

/// Manages the CHIP-8's delay and sound timers, which both count down
/// toward zero at a fixed rate of 60Hz whenever they are non-zero.
class Timer {
public:
    Timer() = default;

    void Reset();

    /// Decrements both timers by 1 if they are greater than zero.
    /// Should be invoked at a steady 60Hz, independent of CPU instruction
    /// throughput.
    void Tick();

    [[nodiscard]] std::uint8_t GetDelay() const noexcept { return delayTimer_; }
    void SetDelay(std::uint8_t value) noexcept { delayTimer_ = value; }

    [[nodiscard]] std::uint8_t GetSound() const noexcept { return soundTimer_; }
    void SetSound(std::uint8_t value) noexcept { soundTimer_ = value; }

    /// True while the sound timer is active and a tone should be playing.
    [[nodiscard]] bool IsSounding() const noexcept { return soundTimer_ > 0; }

private:
    std::uint8_t delayTimer_ = 0;
    std::uint8_t soundTimer_ = 0;
};

}  // namespace chip8
