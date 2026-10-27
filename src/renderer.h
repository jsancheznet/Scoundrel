#pragma once

#include <vector>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "typedefs.h"
#include "camera.h"

#include "card.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 1.0f}
#define ORANGE   color{1.0f, 0.647f, 0.0f, 1.0f}

#define MAX_SPRITE_COUNT 10000
#define MAX_GLYPH_COUNT 10000
#define MAX_CARD_COUNT 1024

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
    glm::vec2 Scale;
    f32 Rotation;
    rect SrcRect;
    glm::vec4 Tint;
};

struct character_glyph
{
    u64 TextureHandle;
    glm::vec2 ScreenPosition;
    glm::vec4 Rect; // Atlas UVs (left, bottom, right, top)
    glm::vec2 Size;
    glm::vec4 Color;
};

struct card_instance
{
    u64 FrontTexture;   // Bindless handles first, same layout idea as sprite_instance
    u64 BackTexture;
    glm::mat4 Model;    // Built on the CPU, the shader just multiplies by it
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

    u32 CardVAO;
    u32 CardVBO;
    u32 CardEBO;
    u32 CardInstanceVBO;

    u32 CameraUBO;

    // Shaders
    u32 SpriteShader;
    u32 TextShader;
    u32 CardShader;

    std::vector<sprite_instance> Sprites;
    std::vector<character_glyph> Glyphs;
    std::vector<card_instance> CardInstances;

    void Init(SDL_Window *SDLWindow, u32 Width, u32 Height);
    void UpdateViewport(i32 Width, i32 Height);

    void UpdateCameraBuffer(camera *Camera);

    void UploadTexture(u8 *Data, i32 Width, i32 Height, u32 *ID, u64 *Handle);
    void DestroyTexture(u32 *ID, u64 *Handle);

    void DrawTexture(u64 AssetHandle, vec3 Position, vec2 Scale, f32 Rotation, rect SrcRect = {0.0f, 0.0f, 1.0f, 1.0f}, glm::vec4 Tint = {1.0f, 1.0f, 1.0f, 0.0f});
    void DrawText(u64 Font, f32 X, f32 Y, f32 Size, const char *Text, color Color = {1.0f, 1.0f, 1.0f, 1.0f});
    void DrawCard(card &Card);

    void ClearScreen(color Color);
    void EndFrame();

  private:

    void ConfigureRenderer();

    void LoadShaders();

    void CreateCameraUBO();

    void CreateQuadMesh();
    void CreateSpriteMesh();
    void CreateTextMesh();
    void CreateCardMesh();

    u64 CompileShader(const char *Filename);
    void UseShader(u32 Shader);

    void DrawSprites();
    void DrawTexts();
    void DrawCards();

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
