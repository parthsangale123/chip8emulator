#include <cstdio>
#include <exception>
#include <string>

#include "chip8/Application.hpp"

int main(int argc, char** argv) {
    std::string romPath = "roms/Pong.ch8";
    if (argc > 1) {
        romPath = argv[1];
    }

    try {
        chip8::Application app;
        app.Run(romPath);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }

    return 0;
}
