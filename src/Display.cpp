#include "chip8/Display.hpp"

#include "raylib.h"

namespace chip8 {

void Display::Clear() {
    for (auto& row : pixels_) {
        row.fill(false);
    }
    dirty_ = true;
}

bool Display::DrawSprite(std::uint8_t x, std::uint8_t y, std::uint8_t height,
                          const std::uint8_t* spriteData) {
    bool collision = false;

    for (std::uint8_t row = 0; row < height; ++row) {
        const std::uint8_t spriteByte = spriteData[row];
        const int py = (y + row) % kHeight;

        for (int bit = 0; bit < 8; ++bit) {
            const bool spritePixel = (spriteByte & (0x80 >> bit)) != 0;
            if (!spritePixel) {
                continue;
            }

            const int px = (x + bit) % kWidth;
            const bool before = pixels_[py][px];

            pixels_[py][px] = before != spritePixel;

            if (before && !pixels_[py][px]) {
                collision = true;
            }
        }
    }

    dirty_ = true;
    return collision;
}

bool Display::GetPixel(int x, int y) const {
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) {
        return false;
    }
    return pixels_[y][x];
}

void Display::Render() const {
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (pixels_[y][x]) {
                DrawRectangle(x * kPixelScale, y * kPixelScale, kPixelScale, kPixelScale,
                              RAYWHITE);
            }
        }
    }
}

}  // namespace chip8
