#include <stdio.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "log.h"
#include "../assets/Meshes/card.hpp"
#include "asset_manager.h"
#include "card.h"

extern asset_manager AssetMgr;

void renderer::Init(SDL_Window* SDLWindow, u32 Width, u32 Height)
{
    Window = SDLWindow;
    ViewportWidth = Width;
    ViewportHeight = Height;

    ConfigureRenderer();
    LoadShaders();
    CreateQuadMesh();
    CreateSpriteMesh();
    CreateTextMesh();
    CreateCardMesh();
    CreateCameraUBO();
}

void renderer::UpdateViewport(i32 Width, i32 Height)
{
    ViewportWidth = Width;
    ViewportHeight = Height;
    glViewport(0, 0, Width, Height);
}

void renderer::ClearScreen(color Color)
{
    glClearColor(Color.r, Color.g, Color.b, Color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer::DrawSprites()
{
    UseShader(SpriteShader);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(SpritesVAO);
    if(Sprites.size() != 0)
    {
        glNamedBufferSubData(SpritesVBO, 0, Sprites.size() * sizeof(sprite_instance), &Sprites[0]);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, Sprites.size());
    }
    Sprites.clear();
}

void renderer::DrawTexts()
{
    UseShader(TextShader);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBindVertexArray(TextVAO);
    if(Glyphs.size() != 0)
    {
        glNamedBufferSubData(TextVBO, 0, Glyphs.size() * sizeof(character_glyph), &Glyphs[0]);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, Glyphs.size());
    }
    glDisable(GL_BLEND);
    Glyphs.clear();
}

void renderer::DrawCards()
{
    UseShader(CardShader);
    glEnable(GL_DEPTH_TEST);

    // Each side of the card is its own set of triangles, culling skips the side facing away from the camera.
    // NOTE: A negative scale flips the winding and would cull the wrong side
    glEnable(GL_CULL_FACE);

    glBindVertexArray(CardVAO);
    if(CardInstances.size() != 0)
    {
        glNamedBufferSubData(CardInstanceVBO, 0, CardInstances.size() * sizeof(card_instance), &CardInstances[0]);
        glDrawElementsInstanced(GL_TRIANGLES, std::size(kCardIndices), GL_UNSIGNED_INT, nullptr, CardInstances.size());
    }

    glDisable(GL_CULL_FACE);
    CardInstances.clear();
}

void renderer::EndFrame()
{
    DrawSprites();
    DrawCards();
    DrawTexts(); // Last, text is UI and goes on top of everything
    SDL_GL_SwapWindow(Window);
}

u64 renderer::CompileShader(const char *Filename)
{
    Assert(Filename);

    // Check if the file exists
    b32 FileExists = SDL_GetPathInfo(Filename, NULL);
    if(!FileExists)
    {
        Log(Error, "renderer::CompileShader() - Could not find shader file: %s", Filename);
        return 0;
    }

    Log(Info, "renderer::CompileShader() - Compiling shader: %s", Filename);

    size_t Size;
    char *FileString = static_cast<char*>(SDL_LoadFile(Filename, &Size));

    u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *VertexSource[2] = {"#version 460 core\n#define VERTEX_SHADER\n", FileString};
    glShaderSource(VertexShader, 2, VertexSource, NULL);
    glCompileShader(VertexShader);
    i32 Compiled;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(VertexShader, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        Log(Error, "renderer::CompileShader(%s) - %s", Filename, ErrorMessage);
        VertexShader = 0;
    }

    u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *FragmentSource[2] = {"#version 460 core\n#define FRAGMENT_SHADER\n", FileString};
    glShaderSource(FragmentShader, 2, FragmentSource, NULL);
    glCompileShader(FragmentShader);
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Compiled);
    if (Compiled != GL_TRUE)
    {
        i32 LogLength = 0;
        char ErrorMessage[1024];
        glGetShaderInfoLog(FragmentShader, 1024, &LogLength, ErrorMessage);
        fprintf(stderr, "%s-%s\n", Filename, ErrorMessage);
        FragmentShader = 0;
    }

    u32 CompiledShader = glCreateProgram();
    glAttachShader(CompiledShader, VertexShader);
    glAttachShader(CompiledShader, FragmentShader);
    glLinkProgram(CompiledShader);
    i32 IsLinked = 0;
    glGetProgramiv(CompiledShader, GL_LINK_STATUS, (GLint *)&IsLinked);
    if (IsLinked == GL_FALSE)
    {
        i32 MaxLogLength = 1024;
        char InfoLog[1024] = {0};
        glGetProgramInfoLog(CompiledShader, MaxLogLength, &MaxLogLength, &InfoLog[0]);
        printf("%s: SHADER PROGRAM FAILED TO COMPILE/LINK\n", Filename);
        printf("%s\n", InfoLog);
        glDeleteProgram(CompiledShader);
        CompiledShader = 0;
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    SDL_free(FileString);

    return CompiledShader;
}

void renderer::UploadTexture(u8 *Data, i32 Width, i32 Height, u32 *ID, u64 *Handle)
{
    Assert(Data && ID && Handle);
    Assert(Width > 0 && Height > 0);

    glCreateTextures(GL_TEXTURE_2D, 1, ID);

    glTextureParameteri(*ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(*ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(*ID, 1, GL_RGBA8, Width, Height);

    glTextureSubImage2D(*ID, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, Data);

    glGenerateTextureMipmap(*ID);

    *Handle = glGetTextureHandleARB(*ID);

    glMakeTextureHandleResidentARB(*Handle);

    Log(Info, "renderer::UploadTexture()");
}

void renderer::DestroyTexture(u32 *ID, u64 *Handle)
{
    Assert(ID && Handle);

    // A resident bindless handle has to be made non resident before deleting its texture
    if(*Handle) glMakeTextureHandleNonResidentARB(*Handle);
    if(*ID) glDeleteTextures(1, ID);

    *ID = 0;
    *Handle = 0;

    Log(Info, "renderer::DeleteTexture()");
}

void renderer::DrawTexture(u64 AssetHandle, vec3 Position, vec2 Scale, f32 Rotation, rect SrcRect, glm::vec4 Tint)
{
    texture *Texture = (texture*)AssetMgr.ResolveHandle(AssetHandle);
    if(Texture == nullptr)
    {
        Log(Warning, "renderer::DrawTexture() - Tried to draw an invalid texture handle");
        return;
    }

    sprite_instance Sprite = {};

    Sprite.Position = Position;
    Sprite.Scale = Scale;
    Sprite.Rotation = Rotation;
    Sprite.TextureHandle = Texture->BindlessTextureHandle;
    Sprite.SrcRect = SrcRect;
    Sprite.Tint = Tint;

    Sprites.push_back(Sprite);
}

// Angle is in radians, Axis doesn't need to be normalized (glm::rotate normalizes it)
void renderer::DrawCard(card &Card)
{
    texture *Front = (texture*)AssetMgr.ResolveHandle(Card.Texture);
    texture *Back = (texture*)AssetMgr.ResolveHandle(Card.BackTexture);
    if(Front == nullptr || Back == nullptr)
    {
        Log(Warning, "renderer::DrawCard() - Tried to draw a card with an invalid texture handle");
        return;
    }

    if(CardInstances.size() >= MAX_CARD_COUNT)
    {
        Log(Warning, "renderer::DrawCard() - Reached MAX_CARD_COUNT (%d), card will not be drawn", MAX_CARD_COUNT);
        return;
    }

    // Model = Translate * Rotate * Scale, the card is scaled first, then rotated, then moved into place
    glm::mat4 Model = glm::translate(glm::mat4(1.0f), Card.Position);
    Model = glm::rotate(Model, Card.Angle, Card.Axis);
    // Model = glm::scale(Model, Card.Scale);
    Model = glm::scale(Model, glm::vec3(CardMeshScale));

    card_instance CardInst = {};
    CardInst.FrontTexture = Front->BindlessTextureHandle;
    CardInst.BackTexture = Back->BindlessTextureHandle;
    CardInst.Model = Model;

    CardInstances.push_back(CardInst);
}

// X, Y is the top-left of the text block, Size is the font size (em size) in pixels
void renderer::DrawText(u64 Font, f32 X, f32 Y, f32 Size, const char *Text, color Color)
{
    bitmap_font *BitmapFont = (bitmap_font*)AssetMgr.ResolveHandle(Font);
    if(BitmapFont == nullptr)
    {
        Log(Warning, "renderer:DrawText() - Tried to draw text using an invalid font handle.");
        return;
    }

    if(!Text) return;

    f32 PenX     = X;
    f32 Baseline = Y + BitmapFont->Metrics.Ascender * Size; // Y is the top of the line
    f32 AtlasWidth  = (f32)BitmapFont->Atlas.Width;
    f32 AtlasHeight = (f32)BitmapFont->Atlas.Height;

    for(const char *C = Text; *C; ++C)
    {
        if(*C == '\n')
        {
            PenX = X;
            Baseline += BitmapFont->Metrics.LineHeight * Size;
            continue;
        }

        font_glyph *Glyph = GetGlyph(BitmapFont, *C);
        if(Glyph == nullptr)
        {
            // Fonts generated without a space glyph still need the gap between words
            if(*C == ' ') { PenX += 0.25f * Size; continue; }

            Log(Warning, "renderer::DrawText() - Character '%c' is not in the font", *C);
            continue;
        }

        font_glyph_bounds *Plane = &Glyph->PlaneBounds;
        font_glyph_bounds *Atlas = &Glyph->AtlasBounds;

        // Whitespace glyphs have no bounds, only advance the pen
        if(Plane->Right > Plane->Left && Plane->Top > Plane->Bottom)
        {
            if(Glyphs.size() >= MAX_GLYPH_COUNT)
            {
                Log(Warning, "renderer::DrawText() - Reached MAX_GLYPH_COUNT (%d), text will be truncated", MAX_GLYPH_COUNT);
                return;
            }

            character_glyph Character = {};
            Character.TextureHandle = BitmapFont->Texture.BindlessTextureHandle;
            // Plane bounds are y-up from the baseline, the screen is y-down
            Character.ScreenPosition = {PenX + Plane->Left * Size, Baseline - Plane->Top * Size};
            Character.Size = glm::vec2((Plane->Right - Plane->Left) * Size, (Plane->Top - Plane->Bottom) * Size);
            Character.Rect = glm::vec4(Atlas->Left  / AtlasWidth, Atlas->Bottom / AtlasHeight,
                                       Atlas->Right / AtlasWidth, Atlas->Top    / AtlasHeight);
            Character.Color = glm::vec4(Color.r, Color.g, Color.b, Color.a);

            Glyphs.push_back(Character);
        }

        PenX += Glyph->Advance * Size;
    }
}

void renderer::CreateCameraUBO()
{
    glCreateBuffers(1, &CameraUBO);
    glNamedBufferData(CameraUBO, sizeof(glm::mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 50, CameraUBO);
}

void renderer::ConfigureRenderer()
{
    gladLoadGL();

    // List extensions
    if (0)
    {
        i32 ExtensionCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
        for (i32 i = 0; i < ExtensionCount; i++)
        {
            const GLubyte *Name = glGetStringi(GL_EXTENSIONS, i);
            printf("%s ", Name);
        }
        printf("\n");
    }

    // Log OpenGL info
    i32 MaxUniformBufferBindings = -1;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MaxUniformBufferBindings);
    Log(Info, "OpenGL Max Uniform Buffer Bindings: %d", MaxUniformBufferBindings);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugCallback, nullptr);
#endif

    glViewport(0, 0, ViewportWidth, ViewportHeight);

    // PERF: Opaque sprites draw with blending OFF (the card JPGs have no alpha) and with
    // depth testing ON, so occluded fragments are discarded by early-Z instead of being
    // read-modify-written. This collapses the per-pixel overdraw from ~hundreds down to ~1.
    // The blend func stays configured; re-enable GL_BLEND per-batch for translucent sprites /
    // SDF text, drawn after the opaque pass (ideally with glDepthMask(GL_FALSE)).
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void renderer::LoadShaders()
{
    SpriteShader = CompileShader("shaders/batched_texture.glsl");
    TextShader = CompileShader("shaders/text_sdf.glsl");
    CardShader = CompileShader("shaders/card.glsl");
}

void renderer::CreateQuadMesh()
{
    float Vertices[] =
        {
            // x     y     z      u     v
            -0.5f, -0.5f, 0.0f,  0.0f,  0.0f,  // bottom-left
            0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  // bottom-right
            0.5f,  0.5f, 0.0f,  1.0f,  1.0f,  // top-right
            -0.5f,  0.5f, 0.0f,  0.0f,  1.0f   // top-left
        };

    // Create and allocate Buffer Storage, each VAO binds it as its per vertex data
    glCreateBuffers(1, &QuadVBO);
    glNamedBufferStorage(QuadVBO, sizeof(Vertices), Vertices, GL_DYNAMIC_STORAGE_BIT);
    Log(Info, "OPENGL, Allocating %zu bytes to QuadVBO", sizeof(Vertices));
}

void renderer::CreateSpriteMesh()
{
    glCreateVertexArrays(1, &SpritesVAO);

    Sprites.reserve(MAX_SPRITE_COUNT);

    { // Bind the QuadVBO as vertex data

        // Bind the QuadVBO to binding point 0
        u32 BindingPoint = 0;
        glVertexArrayVertexBuffer(SpritesVAO, BindingPoint, QuadVBO, 0, sizeof(f32) * 5); // 5 floats

        // Vertex Attribute - Configure Vertex Attribute 0 (Position) from the interleaved buffer data
        glEnableVertexArrayAttrib(SpritesVAO, 0);
        glVertexArrayAttribFormat(SpritesVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(SpritesVAO, 0, 0);

        // UV Attribute - Configure Vertex Attribute 1 (UV) from the interleaved buffer data
        glEnableVertexArrayAttrib(SpritesVAO, 1);
        glVertexArrayAttribFormat(SpritesVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 3);
        glVertexArrayAttribBinding(SpritesVAO, 1, 0);
    }

    glCreateBuffers(1, &SpritesVBO);
    u32 BufferSize = sizeof(sprite_instance) * MAX_SPRITE_COUNT;
    glNamedBufferStorage(SpritesVBO, BufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
    Log(Info, "renderer::Init() - Allocating %d bytes to SpritesVBO", BufferSize);

    u32 BindingPoint = 3;
    glVertexArrayVertexBuffer(SpritesVAO, BindingPoint, SpritesVBO, 0, sizeof(sprite_instance));

    // Position
    glEnableVertexArrayAttrib(SpritesVAO, 2);
    glVertexArrayAttribFormat(SpritesVAO, 2, 3, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Position));
    glVertexArrayAttribBinding(SpritesVAO, 2, BindingPoint);

    // Scale
    glEnableVertexArrayAttrib(SpritesVAO, 3);
    glVertexArrayAttribFormat(SpritesVAO, 3, 2, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Scale));
    glVertexArrayAttribBinding(SpritesVAO, 3, BindingPoint);

    // Rotation
    glEnableVertexArrayAttrib(SpritesVAO, 4);
    glVertexArrayAttribFormat(SpritesVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Rotation));
    glVertexArrayAttribBinding(SpritesVAO, 4, BindingPoint);

    // Texture Handle
    glEnableVertexArrayAttrib(SpritesVAO, 5);
    glVertexArrayAttribIFormat(SpritesVAO, 5, 2, GL_UNSIGNED_INT, offsetof(sprite_instance, TextureHandle));
    glVertexArrayAttribBinding(SpritesVAO, 5, BindingPoint);

    // Src Rect
    glEnableVertexArrayAttrib(SpritesVAO, 6);
    glVertexArrayAttribFormat(SpritesVAO, 6, 4, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, SrcRect));
    glVertexArrayAttribBinding(SpritesVAO, 6, BindingPoint);

    // Tint
    glEnableVertexArrayAttrib(SpritesVAO, 7);
    glVertexArrayAttribFormat(SpritesVAO, 7, 4, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Tint));
    glVertexArrayAttribBinding(SpritesVAO, 7, BindingPoint);

    glVertexArrayBindingDivisor(SpritesVAO, 3, 1);
}

void renderer::CreateTextMesh()
{
    glCreateVertexArrays(1, &TextVAO);

    Glyphs.reserve(MAX_GLYPH_COUNT);

    // VBO Creation
    glCreateBuffers(1, &TextVBO);
    u32 BufferSize = sizeof(character_glyph) * MAX_GLYPH_COUNT;
    glNamedBufferStorage(TextVBO, BufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
    Log(Info, "renderer::Init() - Allocating %d bytes to TextVBO", BufferSize);

    glVertexArrayVertexBuffer(TextVAO, 1, TextVBO, 0, sizeof(character_glyph));

    { // Bind the QuadVBO as vertex data

        // Bind the recently created VBO to binding point 0
        u32 BindingPoint = 0;
        glVertexArrayVertexBuffer(TextVAO, BindingPoint, QuadVBO, 0, sizeof(f32) * 5); // 5 floats

        // Vertex Attribute - Configure Vertex Attribute 0 (Position) from the interleaved buffer data
        glEnableVertexArrayAttrib(TextVAO, 0);
        glVertexArrayAttribFormat(TextVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(TextVAO, 0, 0);

        // UV Attribute - Configure Vertex Attribute 1 (UV) from the interleaved buffer data
        glEnableVertexArrayAttrib(TextVAO, 1);
        glVertexArrayAttribFormat(TextVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 3);
        glVertexArrayAttribBinding(TextVAO, 1, 0);
    }

    // Screen Position
    glEnableVertexArrayAttrib(TextVAO, 2);
    glVertexArrayAttribFormat(TextVAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(character_glyph, ScreenPosition));
    glVertexArrayAttribBinding(TextVAO, 2, 1);

    // Rect
    glEnableVertexArrayAttrib(TextVAO, 3);
    glVertexArrayAttribFormat(TextVAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Rect));
    glVertexArrayAttribBinding(TextVAO, 3, 1);

    // Size
    glEnableVertexArrayAttrib(TextVAO, 4);
    glVertexArrayAttribFormat(TextVAO, 4, 2, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Size));
    glVertexArrayAttribBinding(TextVAO, 4, 1);

    // Color
    glEnableVertexArrayAttrib(TextVAO, 5);
    glVertexArrayAttribFormat(TextVAO, 5, 4, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Color));
    glVertexArrayAttribBinding(TextVAO, 5, 1);

    // Texture Handle
    glEnableVertexArrayAttrib(TextVAO, 6);
    glVertexArrayAttribIFormat(TextVAO, 6, 2, GL_UNSIGNED_INT, offsetof(character_glyph, TextureHandle));
    glVertexArrayAttribBinding(TextVAO, 6, 1);

    glVertexArrayBindingDivisor(TextVAO, 1, 1);
}

void renderer::CreateCardMesh()
{
    // Create Buffers
    glCreateVertexArrays(1, &CardVAO);
    glCreateBuffers(1, &CardVBO);
    glCreateBuffers(1, &CardEBO);

    // Upload Vertex Data, format -> f32 Pos[3], f32 Normal[3], f32 Uv[2]
    // The mesh never changes, so the buffers are immutable (no storage flags)
    glNamedBufferStorage(CardVBO, sizeof(kCardVertices), kCardVertices, 0);

    // Upload Index Data
    glNamedBufferStorage(CardEBO, sizeof(kCardIndices), kCardIndices, 0);

    // Vertex Buffer Binding
    glVertexArrayVertexBuffer(CardVAO, 0, CardVBO, 0, sizeof(card_vertex));

    // Index Buffer - the VAO remembers it, so binding the VAO is enough to draw
    glVertexArrayElementBuffer(CardVAO, CardEBO);

    // Position Attribute - Location 0
    glEnableVertexArrayAttrib(CardVAO, 0);
    glVertexArrayAttribFormat(CardVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(card_vertex, Pos));
    glVertexArrayAttribBinding(CardVAO, 0, 0);

    // Normal Attribute - Location 1
    glEnableVertexArrayAttrib(CardVAO, 1);
    glVertexArrayAttribFormat(CardVAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(card_vertex, Normal));
    glVertexArrayAttribBinding(CardVAO, 1, 0);

    // Uv Attribute - Location 2
    glEnableVertexArrayAttrib(CardVAO, 2);
    glVertexArrayAttribFormat(CardVAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(card_vertex, Uv));
    glVertexArrayAttribBinding(CardVAO, 2, 0);

    { // Per instance data, uploaded every frame in DrawCards()

        CardInstances.reserve(MAX_CARD_COUNT);

        glCreateBuffers(1, &CardInstanceVBO);
        u32 BufferSize = sizeof(card_instance) * MAX_CARD_COUNT;
        glNamedBufferStorage(CardInstanceVBO, BufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
        Log(Info, "renderer::CreateCardMesh() - Allocating %d bytes to CardInstanceVBO", BufferSize);

        // Binding point 1 advances once per instance instead of once per vertex
        u32 BindingPoint = 1;
        glVertexArrayVertexBuffer(CardVAO, BindingPoint, CardInstanceVBO, 0, sizeof(card_instance));
        glVertexArrayBindingDivisor(CardVAO, BindingPoint, 1);

        // Model Matrix - Locations 3, 4, 5, 6
        // NOTE: An attribute holds at most 4 floats, so a mat4 takes 4 locations, one per column
        for(u32 Column = 0; Column < 4; ++Column)
        {
            u32 Location = 3 + Column;
            glEnableVertexArrayAttrib(CardVAO, Location);
            glVertexArrayAttribFormat(CardVAO, Location, 4, GL_FLOAT, GL_FALSE, offsetof(card_instance, Model) + Column * sizeof(glm::vec4));
            glVertexArrayAttribBinding(CardVAO, Location, BindingPoint);
        }

        // Front Texture Handle - Location 7
        glEnableVertexArrayAttrib(CardVAO, 7);
        glVertexArrayAttribIFormat(CardVAO, 7, 2, GL_UNSIGNED_INT, offsetof(card_instance, FrontTexture));
        glVertexArrayAttribBinding(CardVAO, 7, BindingPoint);

        // Back Texture Handle - Location 8
        glEnableVertexArrayAttrib(CardVAO, 8);
        glVertexArrayAttribIFormat(CardVAO, 8, 2, GL_UNSIGNED_INT, offsetof(card_instance, BackTexture));
        glVertexArrayAttribBinding(CardVAO, 8, BindingPoint);
    }
}

void renderer::UseShader(u32 Shader)
{
    glUseProgram(Shader);
}

void renderer::UpdateCameraBuffer(camera *Camera)
{
    // Uploads the camera matrices to the CameraMatrices UBO, the matrices are computed by UpdateCamera() in camera.cpp

    glNamedBufferSubData(CameraUBO, 0                    , sizeof(glm::mat4), glm::value_ptr(Camera->Perspective));
    glNamedBufferSubData(CameraUBO, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(Camera->Orthographic));
    glNamedBufferSubData(CameraUBO, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(Camera->View));
}

void renderer::DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam)
{
    const char *_Source;
    const char *_Type;
    const char *_Severity;

    switch(Source)
    {
        case GL_DEBUG_SOURCE_API:
            _Source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _Source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _Source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _Source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _Source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
        default:
            _Source = "UNKNOWN";
            break;
    }

    switch (Type)
    {
        case GL_DEBUG_TYPE_ERROR:
            _Type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _Type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _Type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _Type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _Type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _Type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
        default:
            _Type = "UNKNOWN";
            break;
    }

    switch (Severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            _Severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _Severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _Severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _Severity = "NOTIFICATION";
            break;

        default:
            _Severity = "UNKNOWN";
            break;
    }

    // 131185: OTHER of NOTIFICATION severity, raised from API: Buffer detailed info: Buffer object 2 (bound to GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (3), usage hint is GL_DYNAMIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if(Id == 131185)
        return;

    printf("[OPENGL DEBUG]: %d: %s of %s severity, raised from %s: %s\n", Id, _Type, _Severity, _Source, Message);
}
