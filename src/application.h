#pragma once

#include <SDL3/SDL.h>

#include "typedefs.h"

struct application
{
    b32 IsRunning;

    SDL_Window *Window;
    SDL_GLContext GLContext;

    u32 WindowWidth;
    u32 WindowHeight;

    void Init();
    void CreateWindow(const char *Title, u32 Width, u32 Height);
    void Quit();

    void EndFrame();

    void ProcessEvents();
};
