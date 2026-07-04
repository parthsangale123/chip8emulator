#include "chip8/Keyboard.hpp"

#include "raylib.h"

namespace chip8 {

Keyboard::Keyboard() {
    // CHIP-8 keypad layout mapped onto a modern QWERTY keyboard:
    //
    //   Keypad          Keyboard
    //  1 2 3 C         1 2 3 4
    //  4 5 6 D    ->   Q W E R
    //  7 8 9 E         A S D F
    //  A 0 B F         Z X C V
    keyMap_[0x1] = KEY_ONE;
    keyMap_[0x2] = KEY_TWO;
    keyMap_[0x3] = KEY_THREE;
    keyMap_[0xC] = KEY_FOUR;

    keyMap_[0x4] = KEY_Q;
    keyMap_[0x5] = KEY_W;
    keyMap_[0x6] = KEY_E;
    keyMap_[0xD] = KEY_R;

    keyMap_[0x7] = KEY_A;
    keyMap_[0x8] = KEY_S;
    keyMap_[0x9] = KEY_D;
    keyMap_[0xE] = KEY_F;

    keyMap_[0xA] = KEY_Z;
    keyMap_[0x0] = KEY_X;
    keyMap_[0xB] = KEY_C;
    keyMap_[0xF] = KEY_V;
}

void Keyboard::Update() {
    for (int i = 0; i < kKeyCount; ++i) {
        keyState_[i] = ::IsKeyDown(keyMap_[i]);
    }
}

bool Keyboard::IsKeyDown(std::uint8_t key) const {
    if (key >= kKeyCount) {
        return false;
    }
    return keyState_[key];
}

std::optional<std::uint8_t> Keyboard::GetAnyPressedKey() const {
    for (int i = 0; i < kKeyCount; ++i) {
        if (keyState_[i]) {
            return static_cast<std::uint8_t>(i);
        }
    }
    return std::nullopt;
}

}  // namespace chip8
