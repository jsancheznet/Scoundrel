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

    Renderer.Init(Application.Window, 1366, 768);

    // RECORDATORIO: No irme por las ramas!
    // TODO: Lo mismo para Quads!
    // TODO: Implementar una camara para poder mover y rotar mis triangulos

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    triangle Triangle = Renderer.CreateTriangle();

    Renderer.UseShader(HelloWorldShader);

    Renderer.UpdateCameraUniforms();

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Triangle.Rotation += 0.01f;

        Renderer.ClearScreen(ORANGE);

        Renderer.DrawTriangle(Triangle);

        Renderer.EndFrame();
    }

    Application.Quit();
}
