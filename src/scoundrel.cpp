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

    // TODO: Dibujar una carta con sus dimensiones correctas
    // TODO: Redondear los bordes de la carta, podria hacer un modelo con doble cara y bordes redondeados
	// TODO: Buscar manera de medir performance, asi puedo ver la diferencia entre batched textures o no
    // TODO: Dibujar 100000 cartas en diferentes ubicaciones
    // Tambien podria batchear llamadas de drawtexture todas juntas
    // Podria directamente dibujar una carta!, hacer un modelo de la carta y dibujar eso?
    // Podria crear un arena allocator "GameInstance" allocator y meter mis globales ahi
    // TODO: Dibujar texto

    // PERFORMANCE: el cuello de botella NO son las draw calls (las 10k cartas salen en un
    // solo glDrawArraysInstanced). Es overdraw con blending a 8x MSAA. Camara en z=1, FOV 90,
    // cartas en z=0 -> rango visible y[-1,1] (2 unidades). Cada carta escala 1.0 cubre media
    // pantalla, 10k encimadas = ~1400x overdraw, ~1.5B fragmentos/frame.
    // Resultado tras 1-3: 41 fps (24.4 ms) -> 430 fps (2.3 ms), ~10x. Fixes por impacto:
    // HECHO 1. Quitar MSAA (application.cpp, SAMPLES/BUFFERS a 0). 8x multiplicaba el ancho
    //          de banda del ROP/blend ~8x, casi no aporta visualmente en sprites 2D.
    // HECHO 2. Apagar blending (renderer.cpp, glDisable(GL_BLEND)). El JPG es opaco; el blend
    //          obligaba read-modify-write por fragmento.
    // HECHO 3. Depth buffer + depth test (DEPTH_SIZE 24, glEnable(GL_DEPTH_TEST), limpiar
    //          GL_DEPTH_BUFFER_BIT). Con GL_LESS, aun con todas las cartas en z=0, el early-Z
    //          descarta las capas ocultas (gana la primera por pixel). (Opaco+depth y
    //          alpha-blend son excluyentes por objeto: blendear solo las que necesiten alpha.)
    // TODO: 4. Test realista: escala 1.0 = cartas gigantes. Cartas reales son chicas -> mucho
    //          menos overdraw por si solo.
    // TODO: 5. (Menor, CPU) Dejar de regenerar la SpriteList cada frame (scoundrel.cpp,
    //          ~30k RNG + push_back + re-upload). Generar el layout una vez, subir solo si cambia.
    // TODO: 6. Desactivar el debug GL context + glDebugMessageCallback en release (ver TODO en
    //          application.cpp, y renderer.cpp).

    shader HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");
    texture AwesomeFace = CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");

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
        Renderer.DrawTexture(AwesomeFace, glm::vec3(-0.5f, 0.0f, 0.f), 1.0f, 0.0f);

        for(int i = 0; i < MAX_SPRITE_COUNT - 2;  i++)
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
