# Scoundrel

> **Work in progress**

Implementation of the [Scoundrel](http://stfj.net/art/2011/Scoundrel.pdf) card game.

## Requirements

- **GPU:** OpenGL 4.6 with `GL_ARB_bindless_texture`. All textures are drawn through bindless handles, so the game won't run without it.
  - Supported by most NVIDIA and AMD GPUs and by recent Intel GPUs with up-to-date drivers.
  - Not available on macOS.
  - To check your GPU, look for `GL_ARB_bindless_texture` in the extension list of [GPU Caps Viewer](https://www.geeks3d.com/gpucapsviewer/) (Windows) or `glxinfo | grep bindless` (Linux).
- **Compiler:** Clang. Windows uses `clang-cl` (LLVM) together with the Visual Studio build tools.

All other libraries (SDL3, glad, glm, stb, json.h) are included in `libraries/`.

## Build

### Windows

1. Open a **Developer Command Prompt for Visual Studio** (it sets up the linker and Windows SDK) and make sure `clang-cl` is on the `PATH` (`C:\Program Files\LLVM\bin`).
2. Build SDL3 once, so `SDL3.lib` and `SDL3.dll` end up in `libraries\SDL-release-3.4.8\Release\`:
   ```
   cd libraries\SDL-release-3.4.8
   cmake -S . -B . && cmake --build . --config Release
   ```
3. From the project root run:
   ```
   build.bat
   ```
4. The first time, copy `libraries\SDL-release-3.4.8\Release\SDL3.dll` into `build\`.
5. Run `build\scoundrel.exe` from inside `build\`, so it finds `assets\` and `shaders\`.

### Linux

1. Build SDL3 once, so `libSDL3.so` ends up in `libraries/SDL-release-3.4.8/`:
   ```
   cd libraries/SDL-release-3.4.8
   cmake -S . -B . -DCMAKE_BUILD_TYPE=Release && cmake --build . -j
   ```
2. From the project root run:
   ```
   ./build.sh
   ```
3. Run `./scoundrel` from inside `build/`.

The build scripts copy `assets/` and `src/shaders/` into `build/` on every build.

## Troubleshooting

- **`glad.c` fails with `-Wunused-but-set-global`:** newer Clang versions warn about a variable inside glad, and `-Werror` turns the warning into an error. Add `-Wno-unused-but-set-global` to the compile line in the build script.
- **Windows blocks `scoundrel.exe`** ("blocked by your organization's Device Guard policy"): this is Smart App Control rejecting unsigned programs. It can't be turned off for a single exe, only in Windows Security, under App & browser control.
