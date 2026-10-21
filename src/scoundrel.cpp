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
#include <algorithm>

#include <glm/gtc/matrix_inverse.hpp>

#include <SDL3/SDL.h>

application   Application;
renderer      Renderer;
asset_manager AssetMgr;
mouse         Mouse;
keyboard      Keyboard;
audio_system  Audio;

struct card
{
    f32 Rotation;
    u64 Texture;
    glm::vec3 Position;       // Where the card is drawn

    glm::vec2 Scale;
    glm::vec3 TargetPosition; // Where the card wants to be, Position eases towards it every frame
    glm::vec4 Tint;
};
constexpr glm::vec2 CardSize =  glm::vec2(0.822f, 1.122f);
card CreateCard(u64 Texture, glm::vec3 Position, f32 Rotation, glm::vec3 Scale, glm::vec4 Tint)
{
    card Result = {};

    Result.Texture = Texture;
    Result.Position = Position;
    Result.TargetPosition = Position;
    Result.Rotation = Rotation;
    Result.Scale = CardSize;
    Result.Tint = Tint;

    return Result;
}

b32 CardContainsPoint(card *Card, glm::vec3 Point)
{
    // Point must be on the card plane, returns true if it lands inside the card quad
    glm::vec3 Local = Point - Card->Position;

    // Undo the card rotation (around Z) so we can test against an axis aligned quad
    f32 C = cos(-Card->Rotation);
    f32 S = sin(-Card->Rotation);
    f32 X = C * Local.x - S * Local.y;
    f32 Y = S * Local.x + C * Local.y;

    // Cards use the unit quad (-0.5 to 0.5) scaled by Card->Scale
    glm::vec2 HalfSize = 0.5f * glm::vec2(Card->Scale);
    return glm::abs(X) <= HalfSize.x && glm::abs(Y) <= HalfSize.y;
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

    u64 TexClubs2             = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    u64 Spades5Texture        = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Spades-5.jpg");
    u64 MarcellusFont         = AssetMgr.LoadBitmapFont("assets/Fonts/Marcellus_sc.png", "assets/Fonts/Marcellus_sc.json");

    card Clubs2 = CreateCard(TexClubs2, glm::vec3(-0.5f, 0.0f, 0.f), 0.0f, glm::vec3(0.822f, 1.122f, 0.0f), glm::vec4(0.0f));
    card Spades5 = CreateCard(Spades5Texture, glm::vec3(0.0f, -1.0, 0.0f), 0.0f, glm::vec3(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.45f));

    std::vector<card> Cards;
    Cards.push_back(Clubs2);
    Cards.push_back(Spades5);

    i32 DraggedCard = -1;
    glm::vec3 DragOffset = {}; // Hit point relative to the card center, so the card doesn't snap to the cursor

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

        // TODO(Jsanchez): Turn this whole crad dragging into something i would write instead of what ai did
        // Maybe have a IsDragged on each card, each card has an update function which we call
        // There in card update we move towards the Target Position and do the things we should do,there

        { // Card picking and dragging
            glm::vec3 RayDirection = ScreenToWorldRay(&Mouse, &Renderer, &Camera);
            glm::vec3 CardNormal = glm::vec3(0.0f, 0.0f, 1.0f); // Cards lie flat on the table and only rotate around Z

            b32 MouseDown    = Mouse.IsButtonPressed(SDL_BUTTON_LEFT);
            b32 MouseWasDown = Mouse.WasButtonPressed(SDL_BUTTON_LEFT);

            if(MouseDown && !MouseWasDown) // Just pressed, pick the top card under the cursor
            {
                // NOTE: All cards share the same Z and depth test is GL_LESS, so the first card drawn ends up on top
                for(i32 i = 0; i < (i32)Cards.size(); i++)
                {
                    glm::vec3 Hit;
                    if(RayPlaneIntersection(Camera.Position, RayDirection, Cards[i].Position, CardNormal, &Hit) && CardContainsPoint(&Cards[i], Hit))
                    {
                        DragOffset = Hit - Cards[i].Position;
                        std::rotate(Cards.begin(), Cards.begin() + i, Cards.begin() + i + 1); // Bring it to the top
                        DraggedCard = 0;
                        break;
                    }
                }
            }
            else if(MouseDown && DraggedCard != -1) // Dragging, follow the cursor on the card plane
            {
                card *Card = &Cards[DraggedCard];
                glm::vec3 Hit;
                if(RayPlaneIntersection(Camera.Position, RayDirection, Card->Position, CardNormal, &Hit))
                {
                    Card->TargetPosition.x = Hit.x - DragOffset.x;
                    Card->TargetPosition.y = Hit.y - DragOffset.y;
                }
            }
            else if(!MouseDown) // Released
            {
                DraggedCard = -1;
            }
        }

        { // Cards chase their target position, this makes them lag a bit behind the mouse while dragging
            // NOTE: exp() makes the smoothing frame rate independent, higher FollowSpeed = less lag
            f32 FollowSpeed = 20.0f;
            f32 Blend = 1.0f - exp(-FollowSpeed * (f32)Application.DeltaTime);
            for(card &Card : Cards)
            {
                Card.Position += (Card.TargetPosition - Card.Position) * Blend;
            }
        }

        for(card &Card : Cards)
        {
            Renderer.DrawTexture(Card.Texture, Card.Position, Card.Scale, Card.Rotation, rect{0.0f, 0.0f, 1.0f, 1.0f}, Card.Tint);
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
