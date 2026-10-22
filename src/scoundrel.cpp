#include "SDL3/SDL_scancode.h"
#include "typedefs.h"
#include "application.h"
#include "renderer.h"
#include "asset_manager.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"

#include <stdio.h>
#include <cfloat>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

#include <SDL3/SDL.h>

application   Application;
renderer      Renderer;
asset_manager AssetMgr;
mouse         Mouse;
keyboard      Keyboard;
audio_system  Audio;

enum card_suit
{
    Suit_Clubs,
    Suit_Spades,
    Suit_Diamonds,
    Suit_Hearts,

    Suit_Count
};

enum card_type
{
    Card_Monster,   // Clubs and Spades
    Card_Weapon,    // Diamonds
    Card_Potion,    // Hearts
};

struct card
{
    u64 Texture;        // Front
    u64 BackTexture;
    glm::vec3 Position;
    f32 Angle;          // Radians, rotation around Axis
    glm::vec3 Axis;
    glm::vec2 Scale;    // Size of the card in world units, used for hit detection
    glm::vec4 Tint;

    card_suit Suit;
    card_type Type;
    i32 Value;          // 2-10, J = 11, Q = 12, K = 13, A = 14. Damage for monsters, strength for weapons, health for potions
};

constexpr f32 CardMeshScale = 0.13f; // The card mesh is 6.3 x 8.8 units, this scales it down to the table size
constexpr glm::vec2 CardSize = glm::vec2(6.3f * CardMeshScale, 8.8f * CardMeshScale);
constexpr f32 CardThickness = 0.03f * CardMeshScale; // The card mesh is 0.03 units thick

card CreateCard(u64 Texture, u64 BackTexture, glm::vec3 Position, f32 Angle, glm::vec3 Axis, glm::vec4 Tint)
{
    card Result = {};

    Result.Texture = Texture;
    Result.BackTexture = BackTexture;
    Result.Position = Position;
    Result.Angle = Angle;
    Result.Axis = Axis;
    Result.Scale = CardSize;
    Result.Tint = Tint;

    return Result;
}

// Loads every card used by Scoundrel and returns them as the Dungeon, face down, stacked on DungeonPosition.
// NOTE: Red face cards (J, Q, K) and red Aces are not part of the game, so Hearts and Diamonds only go up to 10
std::vector<card> LoadDungeon(u64 BackTexture, glm::vec3 DungeonPosition)
{
    const char *SuitNames[Suit_Count] = {"Clubs", "Spades", "Diamonds", "Hearts"};
    const char *RankNames[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; // Indexed by Value

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
            card Card = CreateCard(FrontTexture, BackTexture, Position, glm::pi<f32>(), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f));
            Card.Suit = (card_suit)Suit;
            Card.Type = (Suit == Suit_Diamonds) ? Card_Weapon : (Suit == Suit_Hearts) ? Card_Potion : Card_Monster;
            Card.Value = Value;

            Dungeon.push_back(Card);
        }
    }

    Log(Info, "LoadDungeon() - Loaded %zu cards", Dungeon.size());
    Assert(Dungeon.size() == 44);

    return Dungeon;
}

// The card rotation as a matrix, the same rotation DrawCard uses
glm::mat3 CardRotation(card *Card)
{
    return glm::mat3(glm::rotate(glm::mat4(1.0f), Card->Angle, Card->Axis));
}

b32 CardContainsPoint(card *Card, glm::vec3 Point)
{
    // Point must be on the card plane, returns true if it lands inside the card rectangle
    // The card's own width and height directions in world space
    glm::mat3 Rotation = CardRotation(Card);
    glm::vec3 Right = Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 Up    = Rotation * glm::vec3(0.0f, 1.0f, 0.0f);

    // How far the point is from the card center along the card's width and height
    // NOTE: Right and Up are unit length (pure rotation), so the dot products are distances
    glm::vec3 Offset = Point - Card->Position;
    f32 X = glm::dot(Offset, Right);
    f32 Y = glm::dot(Offset, Up);

    glm::vec2 HalfSize = 0.5f * Card->Scale;
    return glm::abs(X) <= HalfSize.x && glm::abs(Y) <= HalfSize.y;
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
        glm::vec3 CardNormal = CardRotation(&Card) * glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 Hit;
        if(RayPlaneIntersection(RayOrigin, RayDirection, Card.Position, CardNormal, &Hit) && CardContainsPoint(&Card, Hit))
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

    camera Camera = CreateCamera();
    Camera.Position = glm::vec3(0.0f, 0.0f, 2.0f);

    u64 CardBackTexture       = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Back.jpg");
    u64 MarcellusFont         = AssetMgr.LoadBitmapFont("assets/Fonts/Marcellus_sc.png", "assets/Fonts/Marcellus_sc.json");

    std::vector<card> Cards = LoadDungeon(CardBackTexture, glm::vec3(-1.5f, 0.0f, 0.0f)); // Dungeon pile on the left, like the table layout in the rules

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
                // TODO: React to the click
                Log(Info, "Clicked card at %.2f, %.2f", ClickedCard->Position.x, ClickedCard->Position.y);
            }
        }

        for(card &Card : Cards)
        {
            Renderer.DrawCard(Card.Texture, Card.BackTexture, Card.Position, glm::vec3(CardMeshScale), Card.Angle, Card.Axis);
        }

        f32 X = 0.0f;
        f32 Y = 0.0f;
        Renderer.DrawText(MarcellusFont, X + 6, Y + 6, 50.0f, "The quick brown fox jumps over the lazy dog.", color{0.0f, 0.0f, 0.0f, 0.6f});
        Renderer.DrawText(MarcellusFont, X,     Y,     50.0f, "The quick brown fox jumps over the lazy dog", HOT_PINK);

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
