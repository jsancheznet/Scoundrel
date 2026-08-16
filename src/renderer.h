#pragma once

#include <vector>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "typedefs.h"

#include "camera.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 0.0f}
#define ORANGE   color{1.0f, 0.647f, 0.0f, 0.0f}

#define MAX_SPRITE_COUNT 10000

using namespace glm;

struct color
{
    f32 r, g, b, a;
};

struct rect
{
    f32 x0, y0, x1, y1;
};

struct texture
{
    u32 ID;
    u64 Handle;
    i32 Width;
    i32 Height;
};

struct sprite_instance
{
    u64 TextureHandle;
    glm::vec3 Position;
    glm::vec3 Scale;
    f32 Rotation;
    rect SrcRect;
    glm::vec4 Tint;
};

struct renderer
{
    SDL_Window* Window;
    u32 ViewportWidth;
    u32 ViewportHeight;

    // GL Buffers
    u32 MainVAO;
    u32 QuadVBO;
    u32 SpritesVBO;
    u32 CameraUBO;

    // Shaders
    u32 CurrentShader;

    std::vector<sprite_instance> SpriteList;

    void Init(SDL_Window *SDLWindow, u32 Width, u32 Height);
    void UpdateViewport(i32 Width, i32 Height);

    shader CompileShader(const char *Filename);

    void UpdateCamera(camera Camera);

    texture CreateTexture(const char *File);
    void DrawTexture(texture Texture, vec3 Position, f32 Scale, f32 Rotation, rect SrcRect = {0.0f, 0.0f, 1.0f, 1.0f}, glm::vec4 Tint = {1.0f, 1.0f, 1.0f, 0.0f});

    void UseShader(u32 Shader);
    void ClearScreen(color Color);
    void EndFrame();


  private:

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
