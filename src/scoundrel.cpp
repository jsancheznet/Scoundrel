#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "texture.h"
#include "random.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application  Application;
renderer     Renderer;
mouse        Mouse;
keyboard     Keyboard;
audio_system Audio;

i32 main(i32 Argc, char **Argv)
{
    // TODO
    //
    // 1- Implementar bindless textures, el programa deberia funcionar asi como esta en este momento
    // 2- Dibujar varias cartas diferentes usando el parametro Texture en DrawTexture
    // 2- En DrawTexture, generar un nuevo parametro para SrcRect, poner valores por defecto en la definicion para dibujar toda la textura

    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    Keyboard.Init();

    Renderer.Init(Application.Window, 1366, 768);

    Audio.Init();
    Audio.SetVolume(1.0f);

    shader HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");
    texture Card = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    texture AwesomeFace = CreateTexture("assets/Textures/awesomeface.png");

    camera Camera = CreateCamera();

    sound TestSong = Audio.CreateSound("assets/Sounds/music.wav");

    Renderer.UseShader(HelloWorldShader);

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
        }

        if(Keyboard.IsPressed(SDL_SCANCODE_O))
        {
            SDL_ResumeAudioDevice(Audio.DeviceID);
        }
        if(Keyboard.IsPressed(SDL_SCANCODE_P))
        {
            SDL_PauseAudioDevice(Audio.DeviceID);
        }

        Renderer.ClearScreen(ORANGE);
        Renderer.UpdateCamera(Camera);

        Renderer.DrawTexture(AwesomeFace, glm::vec3(0.5f, 0.0f, 0.f), 1.0f, 45.0f);
        Renderer.DrawTexture(Card, glm::vec3(-0.5f, 0.0f, 0.f), 1.0f, 0.0f);

        // for(int i = 0; i < MAX_SPRITE_COUNT - 2;  i++)
        // {
        //     f32 x = RandomBetween(-1.0f, 1.0f);
        //     f32 y = RandomBetween(-1.0f, 1.0f);
        //     f32 Rot = RandomBetween(0.0, 365.0f);
        //     Renderer.DrawTexture(AwesomeFace, glm::vec3(x, y, 0.f), 1.0f, Rot);
        // }

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
