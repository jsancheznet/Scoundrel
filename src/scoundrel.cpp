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
    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    Keyboard.Init();

    Renderer.Init(Application.Window, 1366, 768);

    Audio.Init();
    Audio.SetVolume(1.0f);

    // RECORDATORIO: No irme por las ramas!

    // TODO: Do Claude suggested fixes and fix GL Error in logs
    // TODO: Draw 10000 cards and check performance, is instancing doing anything at all?

    // Reproducir sonidos con SDL, y una cancion de fondo, muy bajita
    //    - SDL_OpenAudioDevice() es la primera funcion que debo usar
    // TODO: Dibujar una carta con sus dimensiones correctas
    // TODO: Redondear los bordes de la carta, podria hacer un modelo con doble cara y bordes redondeados
	// TODO: Buscar manera de medir performance, asi puedo ver la diferencia entre batched textures o no
    // TODO: Dibujar 100000 cartas en diferentes ubicaciones
    // Tambien podria batchear llamadas de drawtexture todas juntas
    // Podria directamente dibujar una carta!, hacer un modelo de la carta y dibujar eso?
    // Podria crear un arena allocator "GameInstance" allocator y meter mis globales ahi


    shader HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");
    texture AwesomeFace = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");

    Renderer.MainTexture = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");

    camera Camera = CreateCamera();

    sound MyTestSound = CreateSound("assets/Sounds/music.wav");


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

        Renderer.ClearScreen(ORANGE);
        Renderer.UpdateCamera(Camera);

        Renderer.DrawTexture(AwesomeFace, glm::vec3(0.5f, 0.0f, 0.f), 1.0f, 45.0f);
        Renderer.DrawTexture(AwesomeFace, glm::vec3(-0.5f, 0.0f, 0.f), 1.0f, 0.0f);

        for(int i = 0; i < 10000;  i++)
        {
            f32 x = RandomBetween(-1.0f, 1.0f);
            f32 y = RandomBetween(-1.0f, 1.0f);
            f32 Rot = RandomBetween(0.0, 365.0f);
            Renderer.DrawTexture(AwesomeFace, glm::vec3(x, y, 0.f), 1.0f, Rot);
        }

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
