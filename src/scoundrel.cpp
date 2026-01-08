#include "typedefs.h"
#include "application.h"
#include "renderer.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application Application;
renderer    Renderer;

i32 main(i32 Argc, char **Argv)
{
    Application.Init();
    Application.CreateWindow("Scoundrel", 1366, 768);

    Renderer.Init(Application.Window);

    //
    // Sidequest: Upload to a private and commercial repository online
    //

    // Escribir codigo de uso para crear un triangulo, dandole de comer los vertices
    // Escribir codigo para dibujarlo!

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    f32 TriangleVertices[] =
    {
        0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    u32 TriangleBuffer = Renderer.CreateBuffer(sizeof(TriangleVertices), sizeof(f32) * 3, TriangleVertices);

    Renderer.UseShader(HelloWorldShader);

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Renderer.ClearScreen(ORANGE);



        Renderer.EndFrame();
    }

    Application.Quit();
}
