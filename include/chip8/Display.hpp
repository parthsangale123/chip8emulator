#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace chip8 {

/// Represents the CHIP-8's 64x32 monochrome pixel display and handles
/// rendering it via raylib.
class Display {
public:
    static constexpr int kWidth = 64;
    static constexpr int kHeight = 32;
    static constexpr int kPixelScale = 10;

    Display() = default;

    /// Clears every pixel to off.
    void Clear();

    /// Draws an 8-pixel-wide sprite of the given height starting at (x, y),
    /// XOR-blending it onto the framebuffer. Coordinates wrap around the
    /// edges of the screen. Returns true if any previously-set pixel was
    /// unset as a result (i.e. a collision occurred).
    bool DrawSprite(std::uint8_t x, std::uint8_t y, std::uint8_t height,
                     const std::uint8_t* spriteData);

    [[nodiscard]] bool GetPixel(int x, int y) const;

    /// True if the framebuffer has changed since the dirty flag was last
    /// cleared. Callers may use this to skip redundant redraws.
    [[nodiscard]] bool IsDirty() const noexcept { return dirty_; }
    void ClearDirty() noexcept { dirty_ = false; }

    /// Draws the current framebuffer using raylib. Must be called between
    /// BeginDrawing()/EndDrawing().
    void Render() const;

private:
    std::array<std::array<bool, kWidth>, kHeight> pixels_{};
    bool dirty_ = true;
};

}  // namespace chip8
