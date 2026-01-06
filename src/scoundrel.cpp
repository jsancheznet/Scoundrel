#include "typedefs.h"
#include "application.h"
#include "renderer.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application Application;
renderer Renderer;

i32 main(i32 Argc, char **Argv)
{
    // TODO(Jsanchez): Create simple logging system
    // TODO(Jsanchez): Windowed fullscreen

    Application.Init();
    Application.CreateWindow("Scoundrel", 1366, 768);

    Renderer.Init();

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Renderer.ClearScreen(ORANGE);

        Application.EndFrame();
    }

    Application.Quit();
}
