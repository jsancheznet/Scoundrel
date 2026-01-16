#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "camera.h"
#include "texture.h"

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

    // TODO: Implementar una camara para poder mover y rotar mis texturas

    u32 HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    Renderer.UseShader(HelloWorldShader);

    texture AwesomeFace = CreateTexture("assets/awesomeface.png");

    camera Camera = CreateCamera();

    while(Application.IsRunning)
    {
        Application.ProcessEvents();

        Renderer.ClearScreen(ORANGE);

        Renderer.UpdateCameraUniforms(Camera);

        Renderer.DrawTexture(AwesomeFace, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f);

        Renderer.EndFrame();
    }

    Application.Quit();
}
