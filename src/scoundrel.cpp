#include "typedefs.h"
#include "log.h"
#include "application.h"
#include "renderer.h"
#include "audio.h"
#include "keyboard.h"
#include "mouse.h"
#include "camera.h"
#include "random.h"

#include <stdio.h>

#include <SDL3/SDL.h>

application  Application;
renderer     Renderer;
mouse        Mouse;
keyboard     Keyboard;
audio_system Audio;

struct card
{
    f32 Rotation;
    f32 Scale;
    texture Texture;
    glm::vec3 Position;
    glm::vec4 Tint;
};

card CreateCard(texture Texture, glm::vec3 Position, f32 Rotation, f32 Scale, glm::vec4 Tint)
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
    // TODO
    // - Pasar el directorio de shaders dentro de src, asegurarse que se copie bien el build.sh, agregar todo en build.bat
    // - Object picking usando un nuevo buffer de opengl
    // - Pasar el create texture dentro de renderer
    // - Hacer una prueba de concepto minima para reemplazar Renderer.DrawTexture -> Render::DrawTexture

    // Object Picking
    // DONE: 0- Hacer que todas las cartas tengan un id, el index de array Cards es el ID
    // 1- Crear un framebuffer object para que contenga toda la info de la render pass
    // 2- Crear el renderbuffer y attachearlo al FBO
    // 3- Crear depthbuffer y attachearlo al FBO
    // 4- Crear un vertex shader con input int32 ObjectID, y mat4 MVP
    // 5- Fragment shader convierte el id en color y lo dibuja
    // 6- Bindear el FB, enable depth test, clear los buffers
    // 7- En draw texture agregar el id a otro array
    // 8- Hacer el draw para esto

    Application.Init();

    Application.CreateWindow("Scoundrel", 1366, 768);

    Keyboard.Init();

    Renderer.Init(Application.Window, 1366, 768);

    Audio.Init();
    Audio.SetVolume(1.0f);

    shader HelloWorldShader = Renderer.CompileShader("shaders/hello_world.glsl");

    camera Camera = CreateCamera();
    sound TestSong = Audio.CreateSound("assets/Sounds/music.wav");

    texture TexClubs2           = Renderer.CreateTexture("assets/Textures/Scoundrel-Clubs-2.jpg");
    texture AwesomeFaceTexture    = Renderer.CreateTexture("assets/Textures/awesomeface.png");
    texture Spades5Texture        = Renderer.CreateTexture("assets/Textures/Scoundrel-Spades-5.jpg");

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
