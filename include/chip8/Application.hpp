#pragma once

#include <string>

#include "chip8/Chip8.hpp"
#include "raylib.h"

namespace chip8 {

/// Owns the raylib window/audio device and drives the emulator's main
/// loop: input polling, CPU cycles, timer updates, and rendering.
class Application {
public:
    explicit Application(int pixelScale = Display::kPixelScale);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /// Loads the ROM at `romPath` and runs the main loop until the user
    /// closes the window.
    void Run(const std::string& romPath);

private:
    void HandleInput();
    void UpdateCPU();
    void UpdateAudio();
    void Render();
    void RenderDebugOverlay() const;

    [[nodiscard]] static Wave GenerateBeepWave();

    Chip8 chip8_;

    int pixelScale_;
    int windowWidth_;
    int windowHeight_;

    bool debugMode_ = false;
    bool soundPlaying_ = false;

    // Number of CPU instructions executed per rendered frame. At 60 FPS
    // this yields an effective clock speed of roughly 660Hz, a reasonable
    // default for most CHIP-8 programs.
    int cyclesPerFrame_ = 11;

    Sound beepSound_{};
};

}  // namespace chip8
