#include "typedefs.h"
#include "application.h"
#include "renderer.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application Application;
renderer Renderer;

i32 main(i32 Argc, char **Argv)
{
    // TODO(Jsanchez): Import Miniaud.io, play SFX and song
    // TODO(Jsanchez): Create simple logging system
    // TODO(Jsanchez): Windowed fullscreen toggle on ALT+ENTER

    Application.Init();
    Application.CreateWindow("Scoundrel", 1366, 768);

    Renderer.Init(Application.Window);

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Renderer.ClearScreen(ORANGE);
        Renderer.EndFrame();
    }

    Application.Quit();
}
