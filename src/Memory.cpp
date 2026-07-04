#include "chip8/Memory.hpp"

#include <algorithm>
#include <stdexcept>

#include "chip8/ROMLoader.hpp"

namespace chip8 {

namespace {

// Standard CHIP-8 hexadecimal font set (0-F), 5 bytes per character.
constexpr std::array<std::uint8_t, Memory::kFontSetSize> kFontSet = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

}  // namespace

Memory::Memory() { Reset(); }

void Memory::Reset() {
    data_.fill(0);
    LoadFontSet();
}

void Memory::LoadFontSet() {
    std::copy(kFontSet.begin(), kFontSet.end(), data_.begin() + kFontStart);
}

void Memory::LoadROM(const std::string& path) {
    const std::vector<std::uint8_t> rom = ROMLoader::LoadROM(path);
    LoadROMData(rom.data(), rom.size());
}

void Memory::LoadROMData(const std::uint8_t* data, std::size_t size) {
    if (data == nullptr || size == 0) {
        throw std::runtime_error("ROM data is empty");
    }
    if (static_cast<std::size_t>(kProgramStart) + size > kMemorySize) {
        throw std::runtime_error("ROM is too large to fit in CHIP-8 memory");
    }
    std::copy(data, data + size, data_.begin() + kProgramStart);
}

std::uint8_t Memory::Read(std::uint16_t address) const {
    if (address >= kMemorySize) {
        throw std::out_of_range("Memory read out of range: " + std::to_string(address));
    }
    return data_[address];
}

void Memory::Write(std::uint16_t address, std::uint8_t value) {
    if (address >= kMemorySize) {
        throw std::out_of_range("Memory write out of range: " + std::to_string(address));
    }
    data_[address] = value;
}

}  // namespace chip8
