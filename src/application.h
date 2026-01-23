#pragma once

#include <SDL3/SDL.h>

#include "renderer.h"

#include "typedefs.h"

extern renderer Renderer;

struct application
{
    b32 IsRunning;

    SDL_Window *Window;
    SDL_GLContext GLContext;

    u32 WindowWidth;
    u32 WindowHeight;

    u64 FrameBeginTime;
    u64 FrameEndTime;
    f64 DeltaTime;

    void Init();
    void CreateWindow(const char *Title, u32 Width, u32 Height);
    void Quit();

    void ProcessEvents();

    void BeginFrame();
    void EndFrame();
};
