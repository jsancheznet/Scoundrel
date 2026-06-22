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
    // - Implementar Tinting
    //   - Agregar un tint color a sprite_instance | 5
    //   - Agregar ese tint color en rendere_init cuando se crea el VBO | 5
    //   - GLSL, usar Mix con un tint hardcodeado para verificar que todo funciona, comentar que usamos mix y no aditivo, si queremos bloom aditivo es mejor | 10
    //   - Agregar el tint parameter en DrawTexture con un default que no de tint | 10
    //   - Probar que todo funciona | 10

    // - Implementar deteccion de mouse over sobre una carta, tinteando la seleccionada
    //   - Investigar generalmente se hace, que algoritmo, tal vez pueda estrenar el libro, hablar con claude o buscar blogs
    //   - Clickear sobre la carta tintea de otro color!

    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    Keyboard.Init();

    Renderer.Init(Application.Window, 1366, 768);

    Audio.Init();
    Audio.SetVolume(1.0f);

    shader HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");
    texture Card = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    texture AwesomeFace = CreateTexture("assets/Textures/awesomeface.png");
    texture Spades5 = CreateTexture("assets/Textures/Scoundrel-Spades-5.jpg");
    texture TestingTexture = CreateTexture("assets/Textures/Scoundrel-Spades-9.jpg");

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

        // TODO(Jsanchez): Fix position, probably bad glsl alignment
        Renderer.DrawTexture(Card, glm::vec3(-0.5f, 0.0f, 0.f), 1.0f, 0.0f);
        Renderer.DrawTexture(TestingTexture, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f);
        rect SrcRect = {0.0f, 0.0, 1.0f, 1.0f};
        Renderer.DrawTexture(Spades5, glm::vec3(0.5f, 0.0f, 0.f), 1.0f, 0.0f, SrcRect, {1.0f, 0.0f, 0.0f, 0.5f});

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
