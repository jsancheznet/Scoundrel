#pragma once

#include <SDL3/SDL.h>

#include "typedefs.h"

struct application
{
    b32 IsRunning;

    SDL_Window *Window;
    SDL_GLContext GLContext;

    void Init();
    void CreateWindow(const char *Title, i32 Width, i32 Height);
    void Quit();

    void EndFrame();

    void ProcessEvents();
};
