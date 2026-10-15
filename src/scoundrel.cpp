#include "SDL3/SDL_scancode.h"
#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "asset_manager.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "random.h"
#include "helpers.h"

#include <stdio.h>
#include <iostream>

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
    f32 Scale;
    u64 Texture;
    glm::vec3 Position;
    glm::vec4 Tint;
};

card CreateCard(u64 Texture, glm::vec3 Position, f32 Rotation, f32 Scale, glm::vec4 Tint)
{
    card Result = {};

    Result.Texture = Texture;
    Result.Position = Position;
    Result.Rotation = Rotation;
    Result.Scale = Scale;
    Result.Tint = Tint;

    return Result;
}

glm::vec3 ScreenToWorldRay(f32 ScreenMouseX, f32 ScreenMouseY, u32 ViewportWidth, u32 ViewportHeight, glm::mat4 Perspective, glm::mat4 View)
{
    // TODO(Jsanchez): Tomar nota de los pasos y subirlo a un gist

    // Step 1 - Mouse NDC Coords
    f32 X_NDC = (ScreenMouseX * 2.0f) / ViewportWidth - 1.0f;
    f32 Y_NDC = 1.0f - (2.0f * ScreenMouseY) / ViewportHeight;
    f32 Z_NDC = 1.0f;
    glm::vec3 RayNDC = glm::vec3(X_NDC, Y_NDC, Z_NDC);

    // Step 2 - Homogeneous Clip Coords - Make Ray Z point forwards, Forward is Negative Z on OpenGL
    glm::vec4 RayClip = vec4(RayNDC.x, RayNDC.y, -1.0f, 1.0f);

    // Step 3 - Ray to Eye Space - Multiply by inverse of the projection matrix and Set Z & WW to mean Forwards, and not a point
    glm::vec4 RayEyeSpace = glm::inverse(Perspective) * RayClip;
    RayEyeSpace = glm::vec4(RayEyeSpace.x, RayEyeSpace.y, -1.0f, 0.0f);

    // Step 4 - World Coordinates - Go to world coordinates and normalize the vector to mean a direction
    glm::vec4 RayWorld4 = glm::inverse(View) * glm::vec4(RayEyeSpace.x, RayEyeSpace.y, -1.0f, 0.0f);
    glm::vec3 RayWorld  = glm::normalize(glm::vec3(RayWorld4));

    return RayWorld;
}

i32 main(i32 Argc, char **Argv)
{
    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    AssetMgr.Init();
    Keyboard.Init();
    Renderer.Init(Application.Window, 1366, 768);
    Audio.Init();

    camera Camera = CreateCamera();
    u64 TestSong = AssetMgr.LoadSound("assets/Sounds/music.wav", Channel_Music);
    Audio.Play(TestSong);

    u64 TestSound = AssetMgr.LoadSound("assets/Sounds/SuccesfulClick.wav", Channel_SFX);

    Audio.SetRepeat(TestSound, true);

    u64 TexClubs2             = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    u64 AwesomeFaceTexture    = AssetMgr.LoadTexture("assets/Textures/awesomeface.png");
    u64 Spades5Texture        = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Spades-5.jpg");
    u64 TestingTexture        = AssetMgr.LoadTexture("assets/Textures/awesomeface.png");

    u64 MarcellusFont = AssetMgr.LoadBitmapFont("assets/Fonts/Marcellus_sc.png", "assets/Fonts/Marcellus_sc.json");
    u64 ArialFont     = AssetMgr.LoadBitmapFont("assets/Fonts/Arial.png", "assets/Fonts/Arial.json");

    card Clubs2 = CreateCard(TexClubs2, glm::vec3(-0.5f, 0.0f, 0.f), 0.0f, 1.0f, glm::vec4(0.0f));
    card AwesomeFace = CreateCard(AwesomeFaceTexture, glm::vec3(0.5f, 0.0f, 0.f), 0.0f, 1.0f, glm::vec4(0.1f));
    card Spades5 = CreateCard(Spades5Texture, glm::vec3(0.0f, -1.0, 0.0f), 0.0f, 1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 0.45f));

    std::vector<card> Cards;
    Cards.push_back(Clubs2);
    Cards.push_back(AwesomeFace);
    Cards.push_back(Spades5);

    while(Application.IsRunning)
    {
        Application.BeginFrame();

        Application.ProcessEvents();

        Mouse.Update();
        Keyboard.Update();

        if(Keyboard.IsPressed(SDL_SCANCODE_LALT) && Keyboard.IsReleased(SDL_SCANCODE_RETURN))
        {
            Log(Info, "Yep!");
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

        if(Keyboard.IsReleased(SDL_SCANCODE_O))
        {
            Audio.Play(TestSound);
        }

        if(Keyboard.IsReleased(SDL_SCANCODE_P))
        {
            Audio.Pause(TestSound);
            Audio.Pause(TestSong);
        }

        if(Keyboard.IsReleased(SDL_SCANCODE_X))
        {
            Audio.Resume(TestSound);
            Audio.Resume(TestSong);
        }

        if(Keyboard.IsReleased(SDL_SCANCODE_R))
        {
            Audio.SetRepeat(TestSound, false);
        }

        if(Keyboard.IsReleased(SDL_SCANCODE_J))
        {
            AssetMgr.Unload(AwesomeFaceTexture);
        }

        Renderer.ClearScreen(ORANGE);
        Renderer.UpdateCamera(Camera);

        Renderer.DrawTexture(Clubs2.Texture, Clubs2.Position, Clubs2.Scale, Clubs2.Rotation);
        Renderer.DrawTexture(AwesomeFace.Texture, AwesomeFace.Position, AwesomeFace.Scale, AwesomeFace.Rotation);
        rect SrcRect = {0.0f, 0.0, 1.0f, 1.0f};
        Renderer.DrawTexture(Spades5.Texture, Spades5.Position, Spades5.Scale, Spades5.Rotation, SrcRect, Spades5.Tint);

        glm::vec3 Ray = ScreenToWorldRay(Mouse.X, Mouse.Y, Renderer.ViewportWidth, Renderer.ViewportHeight, Camera.Perspective, Camera.View);
        //char TextBuffer[200];
        //snprintf(TextBuffer, sizeof(TextBuffer), "ScreenToWorldRay - %.2f, %.2f, %.2f", Ray.x, Ray.y, Ray.z);
        //Renderer.DrawText(MarcellusFont, 0.0f, 0.0f, 40.0f, TextBuffer);

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
