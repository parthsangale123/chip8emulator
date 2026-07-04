#include "chip8/Application.hpp"

#include <cmath>
#include <cstdlib>

namespace chip8 {

namespace {
constexpr int kSampleRate = 44100;
constexpr float kBeepFrequency = 440.0f;
constexpr float kBeepDuration = 0.25f;
constexpr int kDebugPanelWidth = 300;
}  // namespace

Application::Application(int pixelScale)
    : pixelScale_(pixelScale),
      windowWidth_(Display::kWidth * pixelScale_),
      windowHeight_(Display::kHeight * pixelScale_) {
    InitWindow(windowWidth_ + kDebugPanelWidth, windowHeight_, "CHIP-8 Emulator");
    SetTargetFPS(60);

    InitAudioDevice();
    const Wave wave = GenerateBeepWave();
    beepSound_ = LoadSoundFromWave(wave);
    UnloadWave(wave);
    SetSoundVolume(beepSound_, 0.35f);
}

Application::~Application() {
    UnloadSound(beepSound_);
    CloseAudioDevice();
    CloseWindow();
}

Wave Application::GenerateBeepWave() {
    Wave wave{};
    wave.frameCount = static_cast<unsigned int>(kSampleRate * kBeepDuration);
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;

    auto* samples = static_cast<short*>(std::malloc(wave.frameCount * sizeof(short)));
    for (unsigned int i = 0; i < wave.frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSampleRate);
        const float value = std::sin(2.0f * PI * kBeepFrequency * t);
        samples[i] = static_cast<short>(value * 3000.0f);
    }
    wave.data = samples;

    return wave;
}

void Application::Run(const std::string& romPath) {
    chip8_.LoadROM(romPath);

    while (!WindowShouldClose()) {
        HandleInput();
        UpdateCPU();
        chip8_.UpdateTimers();
        UpdateAudio();
        Render();
    }
}

void Application::HandleInput() {
    chip8_.GetKeyboard().Update();

    if (IsKeyPressed(KEY_F1)) {
        debugMode_ = !debugMode_;
    }
}

void Application::UpdateCPU() {
    for (int i = 0; i < cyclesPerFrame_; ++i) {
        chip8_.RunCycle();
    }
}

void Application::UpdateAudio() {
    const bool shouldPlay = chip8_.GetTimer().IsSounding();

    if (shouldPlay && !soundPlaying_) {
        PlaySound(beepSound_);
        soundPlaying_ = true;
    } else if (!shouldPlay) {
        soundPlaying_ = false;
    }
}

void Application::Render() {
    BeginDrawing();
    ClearBackground(BLACK);

    chip8_.GetDisplay().Render();
    chip8_.GetDisplay().ClearDirty();

    if (debugMode_) {
        RenderDebugOverlay();
    }

    EndDrawing();
}

void Application::RenderDebugOverlay() const {
    const CPU& cpu = chip8_.GetCPU();

    const int panelX = windowWidth_ + 10;
    int y = 10;
    constexpr int lineHeight = 18;

    DrawRectangle(windowWidth_, 0, kDebugPanelWidth, windowHeight_, DARKGRAY);

    DrawText("DEBUG (F1)", panelX, y, 18, RAYWHITE);
    y += lineHeight * 2;

    DrawText(TextFormat("Opcode: 0x%04X", cpu.GetCurrentOpcode()), panelX, y, 16, GREEN);
    y += lineHeight;
    DrawText(TextFormat("PC: 0x%04X", cpu.GetProgramCounter()), panelX, y, 16, GREEN);
    y += lineHeight;
    DrawText(TextFormat("I: 0x%04X", cpu.GetIndexRegister()), panelX, y, 16, GREEN);
    y += lineHeight;
    DrawText(TextFormat("SP: %d", cpu.GetStackPointer()), panelX, y, 16, GREEN);
    y += lineHeight;
    DrawText(TextFormat("Instructions: %llu",
                         static_cast<unsigned long long>(cpu.GetInstructionsExecuted())),
             panelX, y, 16, GREEN);
    y += lineHeight;
    DrawText(TextFormat("FPS: %d", GetFPS()), panelX, y, 16, GREEN);
    y += lineHeight * 2;

    DrawText("Registers:", panelX, y, 16, YELLOW);
    y += lineHeight;

    const auto& registers = cpu.GetRegisters();
    for (int i = 0; i < CPU::kRegisterCount; i += 2) {
        DrawText(TextFormat("V%X: 0x%02X   V%X: 0x%02X", i, registers[i], i + 1, registers[i + 1]),
                 panelX, y, 14, LIGHTGRAY);
        y += lineHeight;
    }

    y += lineHeight;
    DrawText("Stack:", panelX, y, 16, YELLOW);
    y += lineHeight;

    const auto& stack = cpu.GetStack();
    const int stackDepth = cpu.GetStackPointer();
    for (int i = 0; i < stackDepth; ++i) {
        DrawText(TextFormat("[%d]: 0x%04X", i, stack[i]), panelX, y, 14, LIGHTGRAY);
        y += lineHeight;
    }
}

}  // namespace chip8
