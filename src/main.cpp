#include "typedefs.h"
#include "main.h"
#include "application.h"
#include "renderer.h"
#include "asset_manager.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "card.h"
#include "scoundrel.h"

#include <stdio.h>
#include <cfloat>
#include <algorithm>
#include <ranges>
#include <random>
#include <vector>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

#include <SDL3/SDL.h>
#include "SDL3/SDL_scancode.h"

application   Application;
renderer      Renderer;
asset_manager AssetMgr;
mouse         Mouse;
keyboard      Keyboard;
audio_system  Audio;

game Scoundrel;

// TODO: Find the place, maybe refactor it into SelectRayCast, what does claude think the name should be?
// Returns the card closest to the camera under the ray, nullptr if the ray doesn't hit any card
// NOTE: The pointer is only valid until Cards is resized, use it in the same frame
card *PickCard(std::vector<card> &Cards, glm::vec3 RayOrigin, glm::vec3 RayDirection)
{
    card *Closest = nullptr;
    f32 ClosestDistance = FLT_MAX;

    for(card &Card : Cards)
    {
        // The plane the card lies on, it tilts with the card
        glm::vec3 CardNormal = Card.RotationMatrix() * glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 Hit;
        if(RayPlaneIntersection(RayOrigin, RayDirection, Card.Position, CardNormal, &Hit) && Card.ContainsPoint(Hit))
        {
            // NOTE: Strict < keeps the first card on ties, cards at the same Z are drawn first on top (depth test is GL_LESS)
            f32 Distance = glm::length(Hit - RayOrigin);
            if(Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                Closest = &Card;
            }
        }
    }

    return Closest;
}

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
        Application.ProcessEvents();

        // TODO: Quitar esto una vez lo haya migrado
#if 0
        b32 MouseDown    = Mouse.IsButtonPressed(SDL_BUTTON_LEFT);
        b32 MouseWasDown = Mouse.WasButtonPressed(SDL_BUTTON_LEFT);
        if(MouseDown && !MouseWasDown)
        {
            glm::vec3 RayDirection = ScreenToWorldRay(&Mouse, &Renderer, &Camera);
            card *ClickedCard = PickCard(Scoundrel.Dungeon, Camera.Position, RayDirection);
            if(ClickedCard && ClickedCard->Pile == Pile_Dungeon)
            {
                Log(Info, "Clicked the dungeon");
            }
        }
#endif

        Scoundrel.Update();
        Scoundrel.Render();

        Application.EndFrame();
    }

    Application.Quit();
}
