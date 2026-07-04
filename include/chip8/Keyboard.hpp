#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace chip8 {

/// Represents the CHIP-8's 16-key hexadecimal keypad, mapped onto a
/// modern QWERTY keyboard, and polled via raylib.
class Keyboard {
public:
    static constexpr int kKeyCount = 16;

    Keyboard();

    /// Polls the current state of every mapped key. Should be called once
    /// per frame before the CPU consumes key state.
    void Update();

    [[nodiscard]] bool IsKeyDown(std::uint8_t key) const;

    /// Returns the first CHIP-8 key (0x0-0xF) currently held down, if any.
    /// Used to implement FX0A (wait for key press).
    [[nodiscard]] std::optional<std::uint8_t> GetAnyPressedKey() const;

private:
    std::array<bool, kKeyCount> keyState_{};
    std::array<int, kKeyCount> keyMap_{};
};

}  // namespace chip8
