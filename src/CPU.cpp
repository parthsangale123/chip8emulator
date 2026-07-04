#include "chip8/CPU.hpp"

#include "chip8/Display.hpp"
#include "chip8/Keyboard.hpp"
#include "chip8/Memory.hpp"
#include "chip8/Timer.hpp"

namespace chip8 {

namespace {
constexpr std::uint16_t kProgramStart = 0x200;
}  // namespace

CPU::CPU(Memory& memory, Display& display, Keyboard& keyboard, Timer& timer)
    : memory_(memory),
      display_(display),
      keyboard_(keyboard),
      timer_(timer),
      randomEngine_(std::random_device{}()) {
    Reset();
}

void CPU::Reset() {
    v_.fill(0);
    indexRegister_ = 0;
    pc_ = kProgramStart;
    sp_ = 0;
    stack_.fill(0);
    opcode_ = 0;
    instructionsExecuted_ = 0;
    waitingForKey_ = false;
    waitingForKeyRegister_ = 0;
}

void CPU::Fetch() {
    const std::uint8_t high = memory_.Read(pc_);
    const std::uint8_t low = memory_.Read(static_cast<std::uint16_t>(pc_ + 1));
    opcode_ = static_cast<std::uint16_t>((high << 8) | low);
    pc_ = static_cast<std::uint16_t>(pc_ + 2);
}

void CPU::Cycle() {
    if (waitingForKey_) {
        if (const auto key = keyboard_.GetAnyPressedKey()) {
            v_[waitingForKeyRegister_] = *key;
            waitingForKey_ = false;
        } else {
            // Remain paused on this instruction until a key is pressed.
            return;
        }
    }

    Fetch();
    Execute();
    ++instructionsExecuted_;
}

void CPU::Execute() {
    switch ((opcode_ & 0xF000) >> 12) {
        case 0x0: Execute0(opcode_); break;
        case 0x1: Execute1(opcode_); break;
        case 0x2: Execute2(opcode_); break;
        case 0x3: Execute3(opcode_); break;
        case 0x4: Execute4(opcode_); break;
        case 0x5: Execute5(opcode_); break;
        case 0x6: Execute6(opcode_); break;
        case 0x7: Execute7(opcode_); break;
        case 0x8: Execute8(opcode_); break;
        case 0x9: Execute9(opcode_); break;
        case 0xA: ExecuteA(opcode_); break;
        case 0xB: ExecuteB(opcode_); break;
        case 0xC: ExecuteC(opcode_); break;
        case 0xD: ExecuteD(opcode_); break;
        case 0xE: ExecuteE(opcode_); break;
        case 0xF: ExecuteF(opcode_); break;
        default: break;
    }
}

void CPU::Execute0(std::uint16_t opcode) {
    switch (opcode & 0x00FF) {
        case 0xE0:  // 00E0 - CLS: clear the display.
            display_.Clear();
            break;
        case 0xEE:  // 00EE - RET: return from a subroutine.
            if (sp_ > 0) {
                --sp_;
                pc_ = stack_[sp_];
            }
            break;
        default:
            // 0NNN - SYS addr: call an RCA 1802 machine routine. Ignored by
            // modern interpreters (including this one).
            break;
    }
}

void CPU::Execute1(std::uint16_t opcode) {
    // 1NNN - JP addr: jump to address NNN.
    pc_ = opcode & 0x0FFF;
}

void CPU::Execute2(std::uint16_t opcode) {
    // 2NNN - CALL addr: call subroutine at NNN.
    if (sp_ < kStackSize) {
        stack_[sp_] = pc_;
        ++sp_;
    }
    pc_ = opcode & 0x0FFF;
}

void CPU::Execute3(std::uint16_t opcode) {
    // 3XNN - SE Vx, byte: skip next instruction if Vx == NN.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t nn = opcode & 0x00FF;
    if (v_[x] == nn) {
        pc_ = static_cast<std::uint16_t>(pc_ + 2);
    }
}

void CPU::Execute4(std::uint16_t opcode) {
    // 4XNN - SNE Vx, byte: skip next instruction if Vx != NN.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t nn = opcode & 0x00FF;
    if (v_[x] != nn) {
        pc_ = static_cast<std::uint16_t>(pc_ + 2);
    }
}

void CPU::Execute5(std::uint16_t opcode) {
    // 5XY0 - SE Vx, Vy: skip next instruction if Vx == Vy.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t y = (opcode & 0x00F0) >> 4;
    if (v_[x] == v_[y]) {
        pc_ = static_cast<std::uint16_t>(pc_ + 2);
    }
}

void CPU::Execute6(std::uint16_t opcode) {
    // 6XNN - LD Vx, byte: set Vx = NN.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    v_[x] = static_cast<std::uint8_t>(opcode & 0x00FF);
}

void CPU::Execute7(std::uint16_t opcode) {
    // 7XNN - ADD Vx, byte: set Vx = Vx + NN (no carry flag).
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    v_[x] = static_cast<std::uint8_t>(v_[x] + (opcode & 0x00FF));
}

void CPU::Execute8(std::uint16_t opcode) {
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t y = (opcode & 0x00F0) >> 4;

    switch (opcode & 0x000F) {
        case 0x0:  // 8XY0 - LD Vx, Vy: Vx = Vy.
            v_[x] = v_[y];
            break;
        case 0x1:  // 8XY1 - OR Vx, Vy: Vx = Vx | Vy.
            v_[x] |= v_[y];
            break;
        case 0x2:  // 8XY2 - AND Vx, Vy: Vx = Vx & Vy.
            v_[x] &= v_[y];
            break;
        case 0x3:  // 8XY3 - XOR Vx, Vy: Vx = Vx ^ Vy.
            v_[x] ^= v_[y];
            break;
        case 0x4: {  // 8XY4 - ADD Vx, Vy: Vx += Vy, VF = carry.
            const std::uint16_t sum = static_cast<std::uint16_t>(v_[x] + v_[y]);
            const std::uint8_t carry = (sum > 0xFF) ? 1 : 0;
            v_[x] = static_cast<std::uint8_t>(sum & 0xFF);
            v_[0xF] = carry;
            break;
        }
        case 0x5: {  // 8XY5 - SUB Vx, Vy: Vx -= Vy, VF = NOT borrow.
            const std::uint8_t noBorrow = (v_[x] >= v_[y]) ? 1 : 0;
            v_[x] = static_cast<std::uint8_t>(v_[x] - v_[y]);
            v_[0xF] = noBorrow;
            break;
        }
        case 0x6: {  // 8XY6 - SHR Vx: Vx >>= 1, VF = shifted-out bit.
            const std::uint8_t shiftedOut = v_[x] & 0x1;
            v_[x] = static_cast<std::uint8_t>(v_[x] >> 1);
            v_[0xF] = shiftedOut;
            break;
        }
        case 0x7: {  // 8XY7 - SUBN Vx, Vy: Vx = Vy - Vx, VF = NOT borrow.
            const std::uint8_t noBorrow = (v_[y] >= v_[x]) ? 1 : 0;
            v_[x] = static_cast<std::uint8_t>(v_[y] - v_[x]);
            v_[0xF] = noBorrow;
            break;
        }
        case 0xE: {  // 8XYE - SHL Vx: Vx <<= 1, VF = shifted-out bit.
            const std::uint8_t shiftedOut = (v_[x] & 0x80) >> 7;
            v_[x] = static_cast<std::uint8_t>(v_[x] << 1);
            v_[0xF] = shiftedOut;
            break;
        }
        default:
            break;
    }
}

void CPU::Execute9(std::uint16_t opcode) {
    // 9XY0 - SNE Vx, Vy: skip next instruction if Vx != Vy.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t y = (opcode & 0x00F0) >> 4;
    if (v_[x] != v_[y]) {
        pc_ = static_cast<std::uint16_t>(pc_ + 2);
    }
}

void CPU::ExecuteA(std::uint16_t opcode) {
    // ANNN - LD I, addr: set I = NNN.
    indexRegister_ = opcode & 0x0FFF;
}

void CPU::ExecuteB(std::uint16_t opcode) {
    // BNNN - JP V0, addr: jump to NNN + V0.
    pc_ = static_cast<std::uint16_t>((opcode & 0x0FFF) + v_[0]);
}

void CPU::ExecuteC(std::uint16_t opcode) {
    // CXNN - RND Vx, byte: Vx = random byte & NN.
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t nn = opcode & 0x00FF;
    const std::uint8_t randomByte = static_cast<std::uint8_t>(randomDistribution_(randomEngine_));
    v_[x] = randomByte & nn;
}

void CPU::ExecuteD(std::uint16_t opcode) {
    // DXYN - DRW Vx, Vy, nibble: draw an N-byte sprite at (Vx, Vy).
    const std::uint8_t x = (opcode & 0x0F00) >> 8;
    const std::uint8_t y = (opcode & 0x00F0) >> 4;
    const std::uint8_t height = opcode & 0x000F;

    const std::uint8_t startX = static_cast<std::uint8_t>(v_[x] % Display::kWidth);
    const std::uint8_t startY = static_cast<std::uint8_t>(v_[y] % Display::kHeight);

    std::array<std::uint8_t, 15> spriteData{};
    for (std::uint8_t row = 0; row < height; ++row) {
        spriteData[row] = memory_.Read(static_cast<std::uint16_t>(indexRegister_ + row));
    }

    const bool collision = display_.DrawSprite(startX, startY, height, spriteData.data());
    v_[0xF] = collision ? 1 : 0;
}

void CPU::ExecuteE(std::uint16_t opcode) {
    const std::uint8_t x = (opcode & 0x0F00) >> 8;

    switch (opcode & 0x00FF) {
        case 0x9E:  // EX9E - SKP Vx: skip next instruction if key Vx is pressed.
            if (keyboard_.IsKeyDown(v_[x])) {
                pc_ = static_cast<std::uint16_t>(pc_ + 2);
            }
            break;
        case 0xA1:  // EXA1 - SKNP Vx: skip next instruction if key Vx is not pressed.
            if (!keyboard_.IsKeyDown(v_[x])) {
                pc_ = static_cast<std::uint16_t>(pc_ + 2);
            }
            break;
        default:
            break;
    }
}

void CPU::ExecuteF(std::uint16_t opcode) {
    const std::uint8_t x = (opcode & 0x0F00) >> 8;

    switch (opcode & 0x00FF) {
        case 0x07:  // FX07 - LD Vx, DT: Vx = delay timer.
            v_[x] = timer_.GetDelay();
            break;
        case 0x0A:  // FX0A - LD Vx, K: block until a key is pressed, store in Vx.
            waitingForKey_ = true;
            waitingForKeyRegister_ = x;
            break;
        case 0x15:  // FX15 - LD DT, Vx: delay timer = Vx.
            timer_.SetDelay(v_[x]);
            break;
        case 0x18:  // FX18 - LD ST, Vx: sound timer = Vx.
            timer_.SetSound(v_[x]);
            break;
        case 0x1E: {  // FX1E - ADD I, Vx: I += Vx.
            const std::uint16_t sum = static_cast<std::uint16_t>(indexRegister_ + v_[x]);
            v_[0xF] = (sum > 0x0FFF) ? 1 : 0;
            indexRegister_ = sum & 0x0FFF;
            break;
        }
        case 0x29:  // FX29 - LD F, Vx: I = address of font sprite for digit Vx.
            indexRegister_ =
                static_cast<std::uint16_t>(Memory::kFontStart + (v_[x] & 0x0F) * Memory::kFontCharSize);
            break;
        case 0x33: {  // FX33 - LD B, Vx: store BCD representation of Vx at I, I+1, I+2.
            const std::uint8_t value = v_[x];
            memory_.Write(indexRegister_, static_cast<std::uint8_t>(value / 100));
            memory_.Write(static_cast<std::uint16_t>(indexRegister_ + 1),
                          static_cast<std::uint8_t>((value / 10) % 10));
            memory_.Write(static_cast<std::uint16_t>(indexRegister_ + 2),
                          static_cast<std::uint8_t>(value % 10));
            break;
        }
        case 0x55:  // FX55 - LD [I], Vx: store V0..Vx in memory starting at I.
            for (int i = 0; i <= x; ++i) {
                memory_.Write(static_cast<std::uint16_t>(indexRegister_ + i),
                              v_[static_cast<std::size_t>(i)]);
            }
            indexRegister_ = static_cast<std::uint16_t>(indexRegister_ + x + 1);
            break;
        case 0x65:  // FX65 - LD Vx, [I]: load V0..Vx from memory starting at I.
            for (int i = 0; i <= x; ++i) {
                v_[static_cast<std::size_t>(i)] =
                    memory_.Read(static_cast<std::uint16_t>(indexRegister_ + i));
            }
            indexRegister_ = static_cast<std::uint16_t>(indexRegister_ + x + 1);
            break;
        default:
            break;
    }
}

}  // namespace chip8
