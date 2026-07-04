#include "chip8/ROMLoader.hpp"

#include <fstream>
#include <stdexcept>

#include "chip8/Memory.hpp"

namespace chip8 {

std::vector<std::uint8_t> ROMLoader::LoadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM file: " + path);
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
        throw std::runtime_error("ROM file is empty: " + path);
    }

    constexpr std::size_t maxRomSize = Memory::kMemorySize - Memory::kProgramStart;
    if (static_cast<std::size_t>(size) > maxRomSize) {
        throw std::runtime_error("ROM file too large to fit in memory: " + path);
    }

    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read ROM file: " + path);
    }

    return buffer;
}

}  // namespace chip8
