#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "camera.h"
#include "texture.h"
#include "keyboard.h"
#include "mouse.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application Application;
renderer    Renderer;
mouse       Mouse;
keyboard    Keyboard;

i32 main(i32 Argc, char **Argv)
{
    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    Keyboard.Init();

    Renderer.Init(Application.Window, 1366, 768);

    // RECORDATORIO: No irme por las ramas!

    // Despues de eso podria traer miniaud.io, cargar y reproducir algun sonido

    // TODO: Mover la camara con las teclas!
    // TODO: Implementar un UBO para la camara, meter todo ahi, actualizarlo una vez por frame o cuando la camara cambie

	// TODO: Buscar manera de medir performance, asi puedo ver la diferencia entre batched textures o no
    // Tambien podria batchear llamadas de drawtexture todas juntas
    // Podria directamente dibujar una carta!, hacer un modelo de la carta y dibujar eso?
    // Handle Window/Viewport Resizing correctly, unreal engine client always mantains the aspect ratio when resized, that's easy!
    // Podria crear un arena allocator "GameInstance" allocator y meter mis globales ahi

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    Renderer.UseShader(HelloWorldShader);

    texture AwesomeFace = CreateTexture("assets/Scoundrel-Clubs-2.jpg");
    camera Camera = CreateCamera();

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Mouse.Update();
        Keyboard.Update();

        Renderer.ClearScreen(ORANGE);
a
        Renderer.UpdateCameraUniforms(Camera);

        Renderer.DrawTexture(AwesomeFace, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f);

        Renderer.EndFrame();
    }

    Application.Quit();
}
