#pragma once

#include <vector>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "typedefs.h"

#include "camera.h"
#include "texture.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 0.0f}
#define ORANGE   color{1.0f, 0.647f, 0.0f, 0.0f}

using namespace glm;

struct color
{
    f32 r, g, b, a;
};

struct sprite_instance
{
    glm::vec3 Position;
    glm::vec3 Scale;
    f32 Rotation;
};

struct renderer
{
    SDL_Window* Window;
    u32 ViewportWidth;
    u32 ViewportHeight;

    // GL Buffers
    u32 MainVAO;
    u32 QuadVBO;
    u32 CardsVBO;
    u32 CameraUBO;

    // Shaders
    u32 CurrentShader;

    texture MainTexture;

    u32 DrawSpriteCount;

    void Init(SDL_Window *SDLWindow, u32 Width, u32 Height);
    void UpdateViewport(i32 Width, i32 Height);

    shader CompileShader(const char *Filename);

    void UpdateCamera(camera Camera);

    void DrawTextureSlow(texture Texture, vec3 Position, f32 Scale, f32 Rotation);
    void DrawTexture(texture Texture, vec3 Position, f32 Scale, f32 Rotation);

    void UseShader(u32 Shader);
    void ClearScreen(color Color);
    void EndFrame();


private:

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
