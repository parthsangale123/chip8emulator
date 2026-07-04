# CHIP-8 Emulator

A complete, modern C++20 implementation of a CHIP-8 interpreter/emulator,
rendered with [raylib](https://www.raylib.com/) and built with CMake.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Folder Structure](#folder-structure)
3. [CPU Architecture](#cpu-architecture)
4. [Memory Layout](#memory-layout)
5. [Fetch-Decode-Execute Cycle](#fetch-decode-execute-cycle)
6. [Opcode Reference](#opcode-reference)
7. [Display Rendering](#display-rendering)
8. [Keyboard Mapping](#keyboard-mapping)
9. [Building](#building)
10. [Running ROMs](#running-roms)
11. [Debug Mode](#debug-mode)
12. [Testing](#testing)
13. [Known Quirks](#known-quirks)

---

## Project Overview

CHIP-8 is an interpreted programming language and virtual machine designed
in the 1970s to make it easy to write games for 8-bit microcomputers. This
project implements a full CHIP-8 virtual machine, split cleanly into
independent, single-responsibility classes:

| Class        | Responsibility                                             |
|--------------|--------------------------------------------------------------|
| `Memory`     | 4KB RAM, font set, ROM loading, bounds-checked reads/writes |
| `Display`    | 64x32 monochrome framebuffer, sprite XOR-drawing, collision |
| `Keyboard`   | 16-key hex keypad state, keyboard mapping, key-wait support |
| `Timer`      | 60Hz delay and sound timers                                 |
| `CPU`        | Registers, stack, PC, and the fetch-decode-execute cycle    |
| `ROMLoader`  | Reads `.ch8` ROM files from disk into memory buffers        |
| `Chip8`      | Facade that owns and wires together all of the above        |
| `Application`| raylib window/audio/input, the main loop, and debug overlay |

None of the core emulation classes (`Memory`, `CPU`, `Display`, `Keyboard`,
`Timer`, `ROMLoader`, `Chip8`) depend on raylib except `Display` and
`Keyboard`, which use it only for pixel drawing and key polling
respectively. `Application` is the only class that owns a raylib window.

## Folder Structure

```
Chip8/
├── CMakeLists.txt        # Build configuration (fetches raylib automatically)
├── README.md             # This file
├── .gitignore
├── include/
│   └── chip8/
│       ├── Application.hpp
│       ├── Chip8.hpp
│       ├── CPU.hpp
│       ├── Display.hpp
│       ├── Keyboard.hpp
│       ├── Memory.hpp
│       ├── ROMLoader.hpp
│       └── Timer.hpp
├── src/
│   ├── Application.cpp
│   ├── Chip8.cpp
│   ├── CPU.cpp
│   ├── Display.cpp
│   ├── Keyboard.cpp
│   ├── main.cpp
│   ├── Memory.cpp
│   ├── ROMLoader.cpp
│   └── Timer.cpp
├── roms/
│   └── IBM_Logo.ch8      # Bundled sample ROM
└── assets/               # Reserved for future assets (fonts, icons, etc.)
```

## CPU Architecture

The `CPU` class implements the classic CHIP-8 register set:

- **16 general-purpose 8-bit registers**, `V0`-`VF`. `VF` doubles as a flag
  register for carry, borrow, and collision results.
- **One 16-bit index register `I`**, used for memory addressing.
- **A 16-bit program counter `PC`**, initialized to `0x200`.
- **An 8-bit stack pointer `SP`** and a **16-entry call stack** of 16-bit
  return addresses, supporting up to 16 levels of subroutine nesting.
- **Delay and sound timers** (owned by the `Timer` class and referenced by
  the CPU), both counting down at 60Hz.

The CPU holds references (not ownership) to `Memory`, `Display`,
`Keyboard`, and `Timer`, which are all owned by the `Chip8` facade. This
keeps the CPU fully testable in isolation and avoids any global mutable
state.

## Memory Layout

CHIP-8 addresses a flat 4096-byte (4KB) address space:

```
0x000 ─────────────────────────────────────────┐
        Reserved (originally the interpreter    │  0x000 - 0x04F
        itself on real hardware)                 │
0x050 ─────────────────────────────────────────┤
        Built-in font set                       │  0x050 - 0x09F
        (16 hex digits, 5 bytes each = 80 bytes)│
0x0A0 ─────────────────────────────────────────┤
        Unused                                  │  0x0A0 - 0x1FF
0x200 ─────────────────────────────────────────┤
        ROM program data                        │  0x200 - 0xFFF
        (loaded by ROMLoader)                    │
0xFFF ─────────────────────────────────────────┘
```

- `Memory::kProgramStart` (`0x200`) is where every ROM is loaded and where
  the program counter starts execution.
- `Memory::kFontStart` (`0x050`) is where the built-in font sprites live;
  `FX29` computes sprite addresses relative to this base.
- `Memory::Read()` / `Memory::Write()` are bounds-checked and throw
  `std::out_of_range` on invalid addresses, which surfaces bugs immediately
  instead of silently corrupting state.

## Fetch-Decode-Execute Cycle

Each call to `CPU::Cycle()` performs one full instruction cycle:

1. **Fetch** — Two consecutive bytes are read from memory at `PC` and
   combined into a single big-endian 16-bit opcode. `PC` is then advanced
   by 2.
2. **Decode** — The opcode's most significant nibble (`opcode & 0xF000`)
   selects one of 16 handler functions (`Execute0` through `ExecuteF`).
   Within ambiguous families (`0`, `8`, `E`, `F`), a secondary switch on
   the low byte or low nibble selects the exact instruction.
3. **Execute** — The selected handler mutates registers, memory, the
   stack, the display, or the timers as required by the instruction.

`FX0A` (wait for key press) is implemented without blocking the host
thread: when triggered, the CPU sets an internal `waitingForKey_` flag.
On every subsequent `Cycle()` call, instead of fetching a new instruction,
the CPU polls the keyboard; once any key is down, it stores that key in
the destination register, clears the flag, and resumes normal fetching on
the next cycle. This keeps the render loop, timers, and window responsive
while a ROM is waiting for input.

## Opcode Reference

All 35 standard CHIP-8 opcodes are implemented in `CPU.cpp`:

| Opcode  | Mnemonic          | Description                                          |
|---------|-------------------|-------------------------------------------------------|
| `00E0`  | `CLS`             | Clear the display                                     |
| `00EE`  | `RET`             | Return from subroutine                                |
| `1NNN`  | `JP addr`         | Jump to address NNN                                   |
| `2NNN`  | `CALL addr`       | Call subroutine at NNN                                |
| `3XNN`  | `SE Vx, byte`     | Skip next instruction if Vx == NN                     |
| `4XNN`  | `SNE Vx, byte`    | Skip next instruction if Vx != NN                     |
| `5XY0`  | `SE Vx, Vy`       | Skip next instruction if Vx == Vy                     |
| `6XNN`  | `LD Vx, byte`     | Set Vx = NN                                           |
| `7XNN`  | `ADD Vx, byte`    | Set Vx = Vx + NN                                      |
| `8XY0`  | `LD Vx, Vy`       | Set Vx = Vy                                           |
| `8XY1`  | `OR Vx, Vy`       | Set Vx = Vx OR Vy                                     |
| `8XY2`  | `AND Vx, Vy`      | Set Vx = Vx AND Vy                                    |
| `8XY3`  | `XOR Vx, Vy`      | Set Vx = Vx XOR Vy                                    |
| `8XY4`  | `ADD Vx, Vy`      | Set Vx += Vy, VF = carry                              |
| `8XY5`  | `SUB Vx, Vy`      | Set Vx -= Vy, VF = NOT borrow                         |
| `8XY6`  | `SHR Vx`          | Set Vx >>= 1, VF = shifted-out bit                    |
| `8XY7`  | `SUBN Vx, Vy`     | Set Vx = Vy - Vx, VF = NOT borrow                     |
| `8XYE`  | `SHL Vx`          | Set Vx <<= 1, VF = shifted-out bit                    |
| `9XY0`  | `SNE Vx, Vy`      | Skip next instruction if Vx != Vy                     |
| `ANNN`  | `LD I, addr`      | Set I = NNN                                           |
| `BNNN`  | `JP V0, addr`     | Jump to NNN + V0                                      |
| `CXNN`  | `RND Vx, byte`    | Set Vx = random byte AND NN                           |
| `DXYN`  | `DRW Vx, Vy, N`   | Draw N-byte sprite at (Vx, Vy), VF = collision         |
| `EX9E`  | `SKP Vx`          | Skip next instruction if key Vx is pressed            |
| `EXA1`  | `SKNP Vx`         | Skip next instruction if key Vx is not pressed        |
| `FX07`  | `LD Vx, DT`       | Set Vx = delay timer                                  |
| `FX0A`  | `LD Vx, K`        | Wait for a key press, store the key in Vx (non-blocking with respect to rendering) |
| `FX15`  | `LD DT, Vx`       | Set delay timer = Vx                                  |
| `FX18`  | `LD ST, Vx`       | Set sound timer = Vx                                  |
| `FX1E`  | `ADD I, Vx`       | Set I += Vx                                           |
| `FX29`  | `LD F, Vx`        | Set I = address of font sprite for digit Vx           |
| `FX33`  | `LD B, Vx`        | Store BCD representation of Vx at I, I+1, I+2         |
| `FX55`  | `LD [I], Vx`      | Store V0..Vx in memory starting at I                  |
| `FX65`  | `LD Vx, [I]`      | Load V0..Vx from memory starting at I                 |

## Display Rendering

The `Display` class owns a `64x32` boolean framebuffer. `DrawSprite()`
implements the standard CHIP-8 XOR blit:

- Sprites are always 8 pixels wide and 1-15 pixels tall.
- Each bit of each sprite byte is XORed onto the framebuffer.
- Coordinates wrap around both screen edges (`% kWidth`, `% kHeight`).
- If any pixel transitions from *on* to *off* as a result of the XOR, the
  method returns `true`; the CPU uses this to set `VF` for collision
  detection, exactly as `DXYN` specifies.
- A `dirty_` flag is set whenever the framebuffer changes (via `Clear()`
  or `DrawSprite()`), so callers can cheaply check `IsDirty()` if they
  want to skip redundant work; `Application` clears it once per frame
  after rendering.

`Display::Render()` iterates the framebuffer and draws a
`kPixelScale x kPixelScale` (10x10) filled rectangle for every lit pixel
using raylib's `DrawRectangle`.

## Keyboard Mapping

The CHIP-8 keypad is a 4x4 hexadecimal grid. `Keyboard` maps it onto the
left-hand side of a QWERTY keyboard in the conventional layout:

```
CHIP-8 Keypad        Keyboard
┌─┬─┬─┬─┐            ┌─┬─┬─┬─┐
│1│2│3│C│            │1│2│3│4│
├─┼─┼─┼─┤            ├─┼─┼─┼─┤
│4│5│6│D│    ──▶      │Q│W│E│R│
├─┼─┼─┼─┤            ├─┼─┼─┼─┤
│7│8│9│E│            │A│S│D│F│
├─┼─┼─┼─┤            ├─┼─┼─┼─┤
│A│0│B│F│            │Z│X│C│V│
└─┴─┴─┴─┘            └─┴─┴─┴─┘
```

`Keyboard::Update()` should be called once per frame; it snapshots the
state of every mapped key via raylib's `IsKeyDown`. `EX9E`/`EXA1` query
individual keys; `FX0A` uses `GetAnyPressedKey()` to find the first key
currently held down.

## Building

### Prerequisites

- CMake 3.16+
- A C++20 compiler (GCC 11+, Clang 13+, or MSVC 2019 16.11+)
- On Linux, the usual raylib build dependencies (X11/Wayland development
  headers, ALSA). On Debian/Ubuntu:

  ```bash
  sudo apt install libgl1-mesa-dev libx11-dev libxcursor-dev \
      libxrandr-dev libxinerama-dev libxi-dev libasound2-dev
  ```

- An internet connection the first time you configure the project, so
  CMake's `FetchContent` can download raylib (unless raylib is already
  installed system-wide, in which case `find_package` will use that
  instead).

### Build Steps

```bash
cd Chip8
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

The resulting binary is placed at `build/bin/Chip8Emulator` (or
`build/bin/Release/Chip8Emulator.exe` on multi-config generators like
Visual Studio). The bundled `roms/` directory is automatically copied next
to the executable after every build.

## Running ROMs

```bash
# From the build output directory:
./bin/Chip8Emulator roms/IBM_Logo.ch8

# Or point it at any other .ch8 file:
./bin/Chip8Emulator /path/to/Pong.ch8
```

If no ROM path is given on the command line, the emulator defaults to
`roms/IBM_Logo.ch8`.

## Debug Mode

Press **F1** at any time to toggle a live debug panel docked to the right
of the display, showing:

- The current opcode
- The program counter (`PC`)
- The index register (`I`)
- The stack pointer (`SP`) and stack contents
- All 16 `V` registers
- Total instructions executed
- Current FPS

## Testing

This emulator has been designed to correctly run the standard CHIP-8
compatibility test suite as well as classic games:

- **IBM Logo** — bundled in `roms/IBM_Logo.ch8`; exercises `CLS`, `LD`,
  `DRW`, and unconditional jumps.
- **Corax89's `test_opcode.ch8`** — exercises arithmetic, logic, and flag
  behavior across every opcode family.
- **Timendus' CHIP-8 Test Suite** — a comprehensive multi-ROM suite
  covering opcode correctness, flag quirks, keypad handling, and display
  behavior. Download it from its GitHub repository and drop the `.ch8`
  files into `roms/`.
- **Pong, Tetris, Breakout, Space Invaders** — classic CHIP-8 games that
  exercise timers, collision detection, and keypad input under real
  gameplay conditions.

To run any of these, place the ROM file in `roms/` (or anywhere on disk)
and pass its path as the command-line argument, as shown above.

## Known Quirks

CHIP-8 has several long-standing behavioral ambiguities between original
COSMAC VIP interpreters and later reimplementations (e.g. SUPER-CHIP).
This emulator follows the most common "modern" convention used by the
majority of contemporary interpreters and test suites:

- `8XY6`/`8XYE` (shift) operate directly on `Vx` and ignore `Vy`.
- `FX55`/`FX65` (register load/store) advance `I` by `X + 1`, matching
  the original COSMAC VIP behavior.
- `BNNN` uses `V0` (not `VX`) as the jump offset register.

If you need to run ROMs that assume the opposite behavior for any of
these quirks, the relevant logic is isolated in `CPU::Execute8` and
`CPU::ExecuteF` in `src/CPU.cpp` and can be adjusted in one place.
