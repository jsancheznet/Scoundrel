#include "application.h"
#include "log.h"

#include <glad/glad.h>

void application::Init()
{
    IsRunning = true;

    // Wayland compositors enforce vsync at the compositor level and ignore SDL_GL_SetSwapInterval(0).
    // Force X11/XWayland so the swap interval is respected during development.
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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
    i32 Interval = 0;
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
                i32 Width = Event.window.data1;
                i32 Height = Event.window.data2;
                Renderer.UpdateViewport(Width, Height);
                Log(Info, "Window Resized, Width: %d, Height %d", Width, Height);
            } break;
            default:
            {
            } break;
        }
    }
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
