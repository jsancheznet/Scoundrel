#include "typedefs.h"
#include "scoundrel.h"
#include "application.h"
#include "renderer.h"
#include "asset_manager.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "card.h"

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

// Game Globals
u64 CardBackTexture;
const glm::vec3 DungeonPosition = glm::vec3(-1.5f, 0.0f, 0.0f);
u32 PlayerHealth = 20;

std::vector<card> LoadDungeon(glm::vec3 DungeonPosition)
{
    const char *SuitNames[Suit_Count] = {"Clubs", "Spades", "Diamonds", "Hearts"};
    const char *RankNames[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; // Indexed by Value

    CardBackTexture = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Back.jpg");

    std::vector<card> Dungeon;
    Dungeon.reserve(44);

    for(i32 Suit = 0; Suit < Suit_Count; ++Suit)
    {
        b32 IsRed = (Suit == Suit_Diamonds || Suit == Suit_Hearts);
        i32 MaxValue = IsRed ? 10 : 14;

        for(i32 Value = 2; Value <= MaxValue; ++Value)
        {
            char Path[128];
            snprintf(Path, sizeof(Path), "assets/Textures/Scoundrel-%s-%s.jpg", SuitNames[Suit], RankNames[Value]);

            u64 FrontTexture = AssetMgr.LoadTexture(Path);

            // Stack the cards, each one slightly above the previous one so the top of the pile is the closest to the camera
            glm::vec3 Position = DungeonPosition + glm::vec3(0.0f, 0.0f, CardThickness * (f32)Dungeon.size());

            // Face down, the back faces the camera
            card Card = CreateCard(FrontTexture, CardBackTexture, Position, glm::pi<f32>(), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f), Pile_Dungeon);
            Card.Suit = (card_suit)Suit;
            Card.Type = (Suit == Suit_Diamonds) ? Card_Weapon : (Suit == Suit_Hearts) ? Card_Potion : Card_Monster;
            Card.Value = Value;

            Dungeon.push_back(Card);
        }
    }

    Log(Info, "LoadDungeon() - Loaded %zu cards", Dungeon.size());
    Assert(Dungeon.size() == 44);

    // Shuffle the cards
    std::random_device RandomDevice;
    std::mt19937 Generator(RandomDevice());
    std::shuffle(Dungeon.begin(), Dungeon.end(), Generator);

    return Dungeon;
}

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

void DrawHealth(u32 Health, u64 Font)
{
    glm::vec2 ScreenPos = glm::vec2(0.0f, 0.0f);
    f32 Size = 40.0f;

    char TextBuffer[128];
    snprintf(TextBuffer, sizeof(TextBuffer), "Health: %d", Health);
    Renderer.DrawText(Font, ScreenPos.x, ScreenPos.y, Size, TextBuffer, RED);
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

    camera Camera = CreateCamera();
    Camera.Position = glm::vec3(0.0f, 0.0f, 2.0f);

    u64 MarcellusFont = AssetMgr.LoadBitmapFont("assets/Fonts/Marcellus_sc.png", "assets/Fonts/Marcellus_sc.json");

    std::vector<card> Cards = LoadDungeon(DungeonPosition);

    while(Application.IsRunning)
    {
        Application.BeginFrame();

        Application.ProcessEvents();

        Mouse.Update();
        Keyboard.Update();

        if(Keyboard.IsPressed(SDL_SCANCODE_LALT) && Keyboard.IsReleased(SDL_SCANCODE_RETURN))
        {
            Application.ToggleFullscreen();
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_D))
        {
            Camera.Position.x += 0.05f;
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_A))
        {
            Camera.Position.x -= 0.05f;
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_SPACE))
        {
            Camera.Position.x = 0.0f;
            Camera.Position.y = 0.0f;
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_W))
        {
            Camera.Position.y += 0.05f;
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_S))
        {
            Camera.Position.y -= 0.05f;
        }

        Renderer.ClearScreen(ORANGE);
        Camera.Update();

        b32 MouseDown    = Mouse.IsButtonPressed(SDL_BUTTON_LEFT);
        b32 MouseWasDown = Mouse.WasButtonPressed(SDL_BUTTON_LEFT);

        if(MouseDown && !MouseWasDown) // Just clicked
        {
            glm::vec3 RayDirection = ScreenToWorldRay(&Mouse, &Renderer, &Camera);
            card *ClickedCard = PickCard(Cards, Camera.Position, RayDirection);
            if(ClickedCard)
            {
                Log(Info, "Clicked card at %.2f, %.2f", ClickedCard->Position.x, ClickedCard->Position.y);
            }
        }

        for(card &Card : Cards)
        {
            Renderer.DrawCard(Card);
        }

        DrawHealth(PlayerHealth, MarcellusFont);

        { // DEBUG
            char Buff[200];
            snprintf(Buff, sizeof(Buff), "Scoundrel - %.0fps", 1.0f / Application.DeltaTime);
            const char *Ptr = &Buff[0];
            SDL_SetWindowTitle(Application.Window, Ptr);
        }

        Renderer.EndFrame();
        Application.EndFrame();
    }

    Application.Quit();
}
