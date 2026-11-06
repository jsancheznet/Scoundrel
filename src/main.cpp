#include "typedefs.h"
#include "application.h"
#include "renderer.h"
#include "asset_manager.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "card.h"
#include "scoundrel.h"

#include <cfloat>
#include <vector>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

#include <SDL3/SDL.h>

application   Application;
renderer      Renderer;
asset_manager AssetMgr;
mouse         Mouse;
keyboard      Keyboard;
audio_system  Audio;

game Scoundrel;

i32 main(i32 Argc, char **Argv)
{
    Application.Init();
    Application.CreateWindow("Scoundrel", 1366, 768);

    AssetMgr.Init();
    Keyboard.Init();
    Mouse.Init();
    Audio.Init();
    Renderer.Init(Application.Window, 1366, 768);

    Scoundrel.Init(&Renderer, &Audio, &Keyboard, &Mouse, &AssetMgr);

    while(Application.IsRunning)
    {
        Application.BeginFrame();

        Scoundrel.Update();
        Scoundrel.Render();

        Application.EndFrame();
    }

    Application.Quit();
}
