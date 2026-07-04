#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <string>

namespace chip8 {

class Memory;
class Display;
class Keyboard;
class Timer;

/// Implements the CHIP-8 CPU: registers, stack, program counter, and the
/// fetch-decode-execute cycle for all 35 standard opcodes.
class CPU {
public:
    static constexpr int kRegisterCount = 16;
    static constexpr int kStackSize = 16;

    CPU(Memory& memory, Display& display, Keyboard& keyboard, Timer& timer);

    /// Resets all registers, the stack, and the program counter to their
    /// power-on state (PC = 0x200).
    void Reset();

    /// Fetches, decodes, and executes exactly one instruction. If the CPU
    /// is blocked waiting for a keypress (FX0A), this checks for a key
    /// press instead of fetching a new instruction.
    void Cycle();

    // --- Debug / introspection accessors -----------------------------
    [[nodiscard]] std::uint16_t GetProgramCounter() const noexcept { return pc_; }
    [[nodiscard]] std::uint16_t GetIndexRegister() const noexcept { return indexRegister_; }
    [[nodiscard]] std::uint8_t GetStackPointer() const noexcept { return sp_; }
    [[nodiscard]] std::uint16_t GetCurrentOpcode() const noexcept { return opcode_; }
    [[nodiscard]] std::uint64_t GetInstructionsExecuted() const noexcept {
        return instructionsExecuted_;
    }
    [[nodiscard]] const std::array<std::uint8_t, kRegisterCount>& GetRegisters() const noexcept {
        return v_;
    }
    [[nodiscard]] const std::array<std::uint16_t, kStackSize>& GetStack() const noexcept {
        return stack_;
    }
    [[nodiscard]] bool IsWaitingForKey() const noexcept { return waitingForKey_; }

private:
    void Fetch();
    void Execute();

    // Opcode handlers, grouped by leading nibble.
    void Execute0(std::uint16_t opcode);
    void Execute1(std::uint16_t opcode);
    void Execute2(std::uint16_t opcode);
    void Execute3(std::uint16_t opcode);
    void Execute4(std::uint16_t opcode);
    void Execute5(std::uint16_t opcode);
    void Execute6(std::uint16_t opcode);
    void Execute7(std::uint16_t opcode);
    void Execute8(std::uint16_t opcode);
    void Execute9(std::uint16_t opcode);
    void ExecuteA(std::uint16_t opcode);
    void ExecuteB(std::uint16_t opcode);
    void ExecuteC(std::uint16_t opcode);
    void ExecuteD(std::uint16_t opcode);
    void ExecuteE(std::uint16_t opcode);
    void ExecuteF(std::uint16_t opcode);

    Memory& memory_;
    Display& display_;
    Keyboard& keyboard_;
    Timer& timer_;

    std::array<std::uint8_t, kRegisterCount> v_{};
    std::uint16_t indexRegister_ = 0;
    std::uint16_t pc_ = 0;
    std::uint8_t sp_ = 0;
    std::array<std::uint16_t, kStackSize> stack_{};

    std::uint16_t opcode_ = 0;
    std::uint64_t instructionsExecuted_ = 0;

    bool waitingForKey_ = false;
    std::uint8_t waitingForKeyRegister_ = 0;

    std::mt19937 randomEngine_;
    std::uniform_int_distribution<int> randomDistribution_{0, 255};
};

}  // namespace chip8
