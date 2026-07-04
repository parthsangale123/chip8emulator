#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace chip8 {

/// Represents the CHIP-8's 4KB addressable memory space, including the
/// built-in font set and ROM loading facilities.
class Memory {
public:
    static constexpr std::size_t kMemorySize = 4096;
    static constexpr std::uint16_t kProgramStart = 0x200;
    static constexpr std::uint16_t kFontStart = 0x050;
    static constexpr std::size_t kFontSetSize = 80;
    static constexpr std::size_t kFontCharSize = 5;

    Memory();

    /// Clears memory and reloads the font set.
    void Reset();

    /// Loads the built-in hexadecimal font set at kFontStart.
    void LoadFontSet();

    /// Reads a ROM file from disk and loads it at kProgramStart.
    /// Throws std::runtime_error on failure.
    void LoadROM(const std::string& path);

    /// Loads raw ROM bytes at kProgramStart.
    /// Throws std::runtime_error if the data does not fit in memory.
    void LoadROMData(const std::uint8_t* data, std::size_t size);

    [[nodiscard]] std::uint8_t Read(std::uint16_t address) const;
    void Write(std::uint16_t address, std::uint8_t value);

    [[nodiscard]] const std::array<std::uint8_t, kMemorySize>& Raw() const noexcept {
        return data_;
    }

private:
    std::array<std::uint8_t, kMemorySize> data_{};
};

}  // namespace chip8
