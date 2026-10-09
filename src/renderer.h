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
#define MAX_GLYPH_COUNT 10000

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
    u64 BindlessTextureHandle;
    i32 Width;
    i32 Height;
    i32 ChannelCount;
    u8 *Data;
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

struct character_glyph
{
    u64 TextureHandle;
    glm::vec2 ScreenPosition;
    glm::vec2 Rect;
    glm::vec2 Size;
    glm::vec3 Color;
};

struct renderer
{
    SDL_Window* Window;
    u32 ViewportWidth;
    u32 ViewportHeight;

    u32 QuadVBO;

    u32 SpritesVAO;
    u32 SpritesVBO;

    u32 TextVAO;
    u32 TextVBO;

    u32 CameraUBO;

    // Shaders
    u32 SpriteShader;
    u32 TextShader;

    std::vector<sprite_instance> Sprites;
    std::vector<character_glyph> Glyphs;

    void Init(SDL_Window *SDLWindow, u32 Width, u32 Height);
    void UpdateViewport(i32 Width, i32 Height);

    void UpdateCamera(camera Camera);

    void UploadTexture(u8 *Data, i32 Width, i32 Height, u32 *ID, u64 *Handle);
    void DestroyTexture(u32 *ID, u64 *Handle);

    void DrawTexture(u64 AssetHandle, vec3 Position, f32 Scale, f32 Rotation, rect SrcRect = {0.0f, 0.0f, 1.0f, 1.0f}, glm::vec4 Tint = {1.0f, 1.0f, 1.0f, 0.0f});
    void DrawText(u64 Font, i32 X, i32 Y, f32 Size, const char *Text);

    void ClearScreen(color Color);
    void EndFrame();

  private:
    u64 CompileShader(const char *Filename);
    void UseShader(u32 Shader);

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
