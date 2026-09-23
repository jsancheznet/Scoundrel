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

#include <stdio.h>

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

i32 main(i32 Argc, char **Argv)
{
    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    AssetMgr.Init();
    Keyboard.Init();
    Renderer.Init(Application.Window, 1366, 768);
    Audio.Init();

    u64 HelloWorldShader = Renderer.CompileShader("shaders/batched_texture.glsl");

    camera Camera = CreateCamera();
    u64 TestSong = AssetMgr.LoadSound("assets/Sounds/music.wav", Channel_Music);
    Audio.Play(TestSong);

    u64 TestSound = AssetMgr.LoadSound("assets/Sounds/SuccesfulClick.wav", Channel_SFX);

    Audio.SetRepeat(TestSound, true);

    u64 TestingHandle = AssetMgr.LoadSound("assets/Sounds/music.wav", Channel_Music);
    sound *MySound = (sound*)AssetMgr.ResolveHandle(TestingHandle);
    AssetMgr.Unload(TestingHandle);
    Log(Info, "Sound Repeats: %x", &MySound->Repeats);
    MySound = (sound*)AssetMgr.ResolveHandle(TestingHandle);

    u64 TexClubs2             = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    u64 AwesomeFaceTexture    = AssetMgr.LoadTexture("assets/Textures/awesomeface.png");
    u64 Spades5Texture        = AssetMgr.LoadTexture("assets/Textures/Scoundrel-Spades-5.jpg");

    u64 TestingTexture = AssetMgr.LoadTexture("assets/Textures/awesomeface.png");

    card Clubs2 = CreateCard(TexClubs2, glm::vec3(-0.5f, 0.0f, 0.f), 0.0f, 1.0f, glm::vec4(0.0f));
    card AwesomeFace = CreateCard(AwesomeFaceTexture, glm::vec3(0.5f, 0.0f, 0.f), 0.0f, 1.0f, glm::vec4(0.1f));
    card Spades5 = CreateCard(Spades5Texture, glm::vec3(0.0f, -1.0, 0.0f), 0.0f, 1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 0.45f));

    Renderer.UseShader(HelloWorldShader);

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
