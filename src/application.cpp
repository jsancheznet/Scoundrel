#include "application.h"

#include <glad/glad.h>

void application::Init()
{
    IsRunning = true;

    // TODO(Jsanchez): Maybe we should init everything!
    SDL_Init(SDL_INIT_VIDEO);

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

void application::CreateWindow(const char *Title, i32 Width, i32 Height)
{
    Window = SDL_CreateWindow(Title, Width, Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    GLContext = SDL_GL_CreateContext(Window);
}

void application::Quit()
{
    SDL_Quit();
}

void application::EndFrame()
{
    SDL_GL_SwapWindow(Window);
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
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            {
                if(Event.key.key == SDLK_ESCAPE)
                {
                    IsRunning = false;
                }

                break;
            }
        }
    }
}
