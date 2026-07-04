#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chip8 {

/// Utility responsible for reading .ch8 ROM files from disk into memory
/// buffers ready to be loaded into the emulator's Memory.
class ROMLoader {
public:
    ROMLoader() = delete;

    /// Reads the entire contents of the ROM file at `path` into a byte
    /// buffer. Throws std::runtime_error if the file cannot be opened,
    /// is empty, or is too large to fit in CHIP-8 memory.
    [[nodiscard]] static std::vector<std::uint8_t> LoadROM(const std::string& path);
};

}  // namespace chip8
