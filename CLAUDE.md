# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

**Build:**
```bash
./build.bat
```

**Run:**
```bash
cd build && ./scoundrel.exe
```

The build script compiles with clang-cl (debug mode: `/Zi /Od /WX`), copies shaders and assets to `build/` (via `xcopy /D` — only newer files are copied), and links against SDL3. Note that `build/shaders/` is a copy of `shaders/` — edit the source in `shaders/` and rebuild.

**Build prerequisite:** `build.bat` links against `libraries/SDL3-3.2.10/build/Debug/SDL3.lib`. SDL3 must be prebuilt at that path before the project will link.

There is no test or lint setup — this is a from-scratch C++ engine and the toolchain is just clang-cl + the build script.

## Architecture

Scoundrel is a card game built on a custom C++ engine using OpenGL 4.6 Core + SDL3.

**Entry point:** `src/scoundrel.cpp` — contains the main game loop, input handling, and asset loading.

**Core subsystems** (each has a `.h`/`.cpp` pair in `src/`):

- **renderer** — OpenGL context, shader compilation, VAO/VBO management, sprite drawing, and camera UBO (binding 50). Sprites are drawn via `glDrawArraysInstanced`; per-instance data (`instance_data`: WorldPosition / Scale / Rotation) lives in `CardsVBO` at vertex-buffer binding 3 with divisor 1. The quad mesh lives in `QuadVBO` at binding 0.
- **application** — SDL3 window and GL context creation, event dispatch, frame timing / delta time.
- **camera** — Perspective camera; holds view/projection matrices updated into the renderer's UBO.
- **texture** — STB Image loading → OpenGL texture with mipmaps.
- **audio** — SDL3 audio device, WAV loading, per-channel streams.
- **keyboard / mouse** — Per-frame input state (current + previous frame) for edge detection.
- **log** — Colored console output; four levels: Info, Warning, Error, Success.
- **random** — Header-only xorshift PRNG (`RandomU32`, `RandomI32`, `RandomBetween`, `RandomBool`). Uses a static `_State`; treat it as a singleton.
- **typedefs.h** — Project-wide type aliases (`u32`, `i32`, `f32`, etc.) and an `Assert` macro.

## Shaders

Shaders live in `shaders/` as single `.glsl` files split by `#ifdef VERTEX_SHADER` / `#ifdef FRAGMENT_SHADER`. GLSL version is 4.6 core. They are compiled at runtime by `renderer::CompileShader()`.

- `hello_world.glsl` — main sprite shader; uses camera UBO at binding 50; instance data carries WorldPosition, Scale, Rotation for batched rendering.
- `text_sdf.glsl` — SDF text rendering with `smoothstep` anti-aliasing.

## Dependencies (vendored in `libraries/`)

| Library | Purpose |
|---------|---------|
| SDL3 3.2.10 | Window, input, audio |
| GLAD | OpenGL loader |
| GLM 1.0.1-light | Math (vectors, matrices) |
| STB | Image loading (`stb_image.h`) |

## Assets

Cards (56 sprites), font atlases (`Arial.png`, `Roboto.png` + JSON metadata), and audio (`music.wav`, `Thump.wav`, `SuccessfulClick.wav`) live in `assets/` and are copied to `build/assets/` at build time.

## Tools

`tools/msdf-atlas-gen/` is the source for generating the SDF font atlases that `text_sdf.glsl` consumes.

## Conventions

- Naming: types are `snake_case` (`renderer`, `audio_system`, `instance_data`); members, functions, and locals are `PascalCase` (`DrawTexture`, `ViewportWidth`).
- Some inline notes and TODOs in `scoundrel.cpp` are written in Spanish — that's intentional, not a translation task.
