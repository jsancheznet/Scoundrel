#include "application.h"
#include "SDL3/SDL_video.h"
#include "log.h"
#include "random.h"

#include <glad/glad.h>

void application::Init()
{
    IsRunning = true;

#if defined(__linux__)
    // Wayland compositors enforce vsync at the compositor level and ignore SDL_GL_SetSwapInterval(0).
    // Force X11/XWayland so the swap interval is respected during development.
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    RandomSeed((u32)SDL_GetPerformanceCounter() | 1); // | 1 so the seed is never 0

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // PERF: MSAA off. With thousands of overlapping full-screen quads, 8x MSAA multiplied
    // ROP/blend bandwidth ~8x for almost no visual gain (interior edges are occluded).
    // Re-enable a low sample count (2-4) for the final game, which draws only a few cards.
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    // TODO(Jsanchez): Disable this on release builds
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
}

void application::CreateWindow(const char *Title, u32 Width, u32 Height)
{
    Window = SDL_CreateWindow(Title, Width, Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    GLContext = SDL_GL_CreateContext(Window);
    WindowWidth = Width;
    WindowHeight = Height;

    Log(Info, "application::CreateWindow() - Created Window, Width: %d, Height: %d", Width, Height);

    // Disable Vsync
    i32 Interval = 1;
    bool VSyncConfigured = SDL_GL_SetSwapInterval(Interval);
    if(!VSyncConfigured)
    {
        Log(Error, "application::Init - Failed to configure Vsync, SDL_GL_SetSwapInterval(%d), %s", Interval, SDL_GetError());
    }
}

void application::Quit()
{
    SDL_Quit();
}

void application::ProcessEvents()
{
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
            case SDL_EVENT_QUIT:
            {
                IsRunning = false;
            } break;
            case SDL_EVENT_KEY_DOWN:
            {
                if(Event.key.key == SDLK_ESCAPE)
                {
                    IsRunning = false;
                }
            } break;
            case SDL_EVENT_WINDOW_RESIZED:
            {
                WindowWidth = Event.window.data1;
                WindowHeight = Event.window.data2;
                Renderer.UpdateViewport(WindowWidth, WindowHeight);
                Log(Info, "Window Resized, Width: %d, Height %d", WindowWidth, WindowHeight);
            } break;
            default:
            {
            } break;
        }
    }
}

void application::ToggleFullscreen()
{
    if(!IsFullscreen)
    {
        // A maximized window ignores resizes and would be saved as the maximized rect
        if(SDL_GetWindowFlags(Window) & SDL_WINDOW_MAXIMIZED)
        {
            SDL_RestoreWindow(Window);
        }

        // Remember where the window was so we can go back to it
        SDL_GetWindowPosition(Window, &WindowedRect.x, &WindowedRect.y);
        SDL_GetWindowSize(Window, &WindowedRect.w, &WindowedRect.h);

        SDL_Rect Display;
        if(!SDL_GetDisplayBounds(SDL_GetDisplayForWindow(Window), &Display))
        {
            Log(Error, "application::ToggleFullscreen() - SDL_GetDisplayBounds failed: %s", SDL_GetError());
            return;
        }

        // Windowed fullscreen: a borderless window covering the monitor, the display mode never changes.
        // NOTE: The extra pixel of height keeps the window from exactly matching the monitor, otherwise
        // the OpenGL driver promotes it to exclusive fullscreen and we get the slow mode switch again.
        SDL_SetWindowBordered(Window, false);
        SDL_SetWindowPosition(Window, Display.x, Display.y);
        SDL_SetWindowSize(Window, Display.w, Display.h + 1);
    }
    else
    {
        SDL_SetWindowBordered(Window, true);
        SDL_SetWindowSize(Window, WindowedRect.w, WindowedRect.h);
        SDL_SetWindowPosition(Window, WindowedRect.x, WindowedRect.y);
    }

    IsFullscreen = !IsFullscreen;

    // The viewport gets updated when SDL_EVENT_WINDOW_RESIZED arrives
}

void application::BeginFrame()
{
    FrameBeginTime = SDL_GetPerformanceCounter();

    // char TitleBuffer[1024];
    // snprintf(TitleBuffer, sizeof(TitleBuffer), "Scoundrel - FPS: %d", (u32)(1.0f / DeltaTime));
    // SDL_SetWindowTitle(Window, (const char*)&TitleBuffer);
}

void application::EndFrame()
{
    FrameEndTime = SDL_GetPerformanceCounter();
    DeltaTime = ((f64)FrameEndTime - (f64)FrameBeginTime) / (f64)SDL_GetPerformanceFrequency();
}
