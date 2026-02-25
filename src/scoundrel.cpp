#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "texture.h"

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

    // RECORDATORIO: No irme por las ramas!
    // Reproducir sonidos con SDL, y una cancion de fondo, muy bajita
    //    - SDL_OpenAudioDevice() es la primera funcion que debo usar
    // TODO: Dibujar una carta con sus dimensiones correctas
    // TODO: Redondear los bordes de la carta, podria hacer un modelo con doble cara y bordes redondeados
	// TODO: Buscar manera de medir performance, asi puedo ver la diferencia entre batched textures o no
    // TODO: Dibujar 100000 cartas en diferentes ubicaciones
    // Tambien podria batchear llamadas de drawtexture todas juntas
    // Podria directamente dibujar una carta!, hacer un modelo de la carta y dibujar eso?
    // Podria crear un arena allocator "GameInstance" allocator y meter mis globales ahi

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");
    texture AwesomeFace = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    camera Camera = CreateCamera();

    Audio.Init();
    Audio.SetVolume(1.0f);
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
            Camera.Position.x += 0.1f;
        }
        if(Keyboard.IsPressed(SDL_SCANCODE_A))
        {
            Camera.Position.x -= 0.1f;
        }
        if(Keyboard.IsPressed(SDL_SCANCODE_SPACE))
        {
            Camera.Position.x = 0.0f;
        }

        // if(Keyboard.IsPressed(SDL_SCANCODE_A))
        // {
        //     Audio.PlaySound(MyTestSound);
        // }

        Renderer.ClearScreen(ORANGE);
        Renderer.UpdateCamera(Camera);
        Renderer.DrawTexture(AwesomeFace, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f);
        Renderer.SwapBuffers();

        Application.EndFrame();
    }

    Application.Quit();
}
