#include <stdio.h>

#include "renderer.h"
#include "log.h"
#include "asset_manager.h"

extern asset_manager AssetMgr;

void renderer::Init(SDL_Window* SDLWindow, u32 Width, u32 Height)
{
    Window = SDLWindow;
    ViewportWidth = Width;
    ViewportHeight = Height;

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

    // Enable Debug Mode
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugCallback, nullptr);

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

    glCreateVertexArrays(1, &SpritesVAO);
    glBindVertexArray(SpritesVAO);

    glCreateVertexArrays(1, &TextVAO);
    glBindVertexArray(TextVAO);

    Sprites.reserve(MAX_SPRITE_COUNT);
    Glyphs.reserve(MAX_GLYPH_COUNT);

    { // Load Shaders
        SpriteShader = CompileShader("shaders/batched_texture.glsl");
        TextShader = CompileShader("shaders/text_sdf.glsl");
    }

    { // Create a Quad mesh
        float Vertices[] =
        {
            // x     y     z      u     v
            -0.5f, -0.5f, 0.0f,  0.0f,  0.0f,  // bottom-left
            0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  // bottom-right
            0.5f,  0.5f, 0.0f,  1.0f,  1.0f,  // top-right
            -0.5f,  0.5f, 0.0f,  0.0f,  1.0f   // top-left
        };

        // Create and allocate Buffer Storage
        glCreateBuffers(1, &QuadVBO);
        glNamedBufferStorage(QuadVBO, sizeof(Vertices), Vertices, GL_DYNAMIC_STORAGE_BIT);
        Log(Info, "OPENGL, Allocating %d bytes to QuadVBO", sizeof(Vertices));

        // Bind the recently created VBO to binding point 0
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

    { // SpritesVBO

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
        glVertexArrayAttribFormat(SpritesVAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Scale));
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

    { // Text Rendering

        //         Binding point 0 (QuadVBO)  ──► location 0  VertexPosition
        //                                    ──► location 1  UV
        //
        //         Binding point 1 (TextVBO)  ──► location 2  ScreenPosition
        //                                    ──► location 3  Rect
        //                                    ──► location 4  Size
        //                                    ──► location 5  Color
        //                                    ──► location 6  TextureHandle
        //

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
        glVertexArrayAttribFormat(TextVAO, 3, 2, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Rect));
        glVertexArrayAttribBinding(TextVAO, 3, 1);

        // Size
        glEnableVertexArrayAttrib(TextVAO, 4);
        glVertexArrayAttribFormat(TextVAO, 4, 2, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Size));
        glVertexArrayAttribBinding(TextVAO, 4, 1);

        // Color
        glEnableVertexArrayAttrib(TextVAO, 5);
        glVertexArrayAttribFormat(TextVAO, 5, 3, GL_FLOAT, GL_FALSE, offsetof(character_glyph, Color));
        glVertexArrayAttribBinding(TextVAO, 5, 1);

        glVertexArrayBindingDivisor(TextVAO, 1, 1);
    }

    { // Camera UBO setup
        glCreateBuffers(1, &CameraUBO);
        glNamedBufferData(CameraUBO, sizeof(glm::mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 50, CameraUBO);
    }
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

void renderer::EndFrame()
{
    // In here we can split things up according to teir material requirements, bind things and call draw

    // Draw Sprites
    UseShader(SpriteShader);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(SpritesVAO);
    if(Sprites.size() != 0)
    {
        glNamedBufferSubData(SpritesVBO, 0, Sprites.size() * sizeof(sprite_instance), &Sprites[0]);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, Sprites.size());
    }
    Sprites.clear();

    // Draw Text
    UseShader(TextShader); // TODO: Fix the glGetUniform bug, by setting a texture
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(TextVAO);
    if(Glyphs.size() != 0)
    {
        glNamedBufferSubData(TextVBO, 0, Glyphs.size() * sizeof(character_glyph), &Glyphs[0]);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, Glyphs.size());
    }
    Glyphs.clear();

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

void renderer::DrawTexture(u64 AssetHandle, vec3 Position, f32 Scale, f32 Rotation, rect SrcRect, glm::vec4 Tint)
{
    texture *Texture = (texture*)AssetMgr.ResolveHandle(AssetHandle);
    if(Texture == nullptr)
    {
        Log(Warning, "renderer::DrawTexture() - Tried to draw an invalid texture handle");
        return;
    }

    sprite_instance Sprite = {};

    Sprite.Position = Position;
    Sprite.Scale = glm::vec3(Scale);
    Sprite.Rotation = Rotation;
    Sprite.TextureHandle = Texture->BindlessTextureHandle;
    Sprite.SrcRect = SrcRect;
    Sprite.Tint = Tint;

    Sprites.push_back(Sprite);
}

void renderer::DrawText(u64 Font, i32 X, i32 Y, f32 Size, const char *Text)
{
    bitmap_font *BitmapFont = (bitmap_font*)AssetMgr.ResolveHandle(Font);
    if(BitmapFont == nullptr)
    {
        Log(Warning, "renderer:DrawText() - Tried to draw text using an invalid font handle.");
        return;
    }

    // font_glyph *Glyph = GetGlyph(BitmapFont, 'J');

    character_glyph Character = {};
    Character.TextureHandle = 0; // TODO: Get the correct texture handle,
    Character.ScreenPosition = {(f32) X, (f32) Y};
    Character.Rect = glm::vec2(0.0f, 0.0f); // TODO: The Uv's are per vertex, we should use the QuadVBO, talk to claude about this
    Character.Size = glm::vec2((f32) Size, (f32) Size);
    Character.Color = glm::vec3(1.0f, 0.0f, 0.0f);

    Glyphs.push_back(Character);
}

void renderer::UseShader(u32 Shader)
{
    glUseProgram(Shader);
    glUniform1i(glGetUniformLocation(Shader, "Texture"), 0);
}

void renderer::UpdateCamera(camera Camera)
{
    Camera.View = glm::lookAt(Camera.Position, Camera.Target, Camera.Up);
    Camera.Perspective = glm::perspective(glm::radians(Camera.Fov), Camera.AspectRatio, Camera.Near, Camera.Far);
    Camera.Orthographic = glm::ortho(0.0f, (f32)ViewportWidth, (f32)ViewportHeight, 0.0f);

    glNamedBufferSubData(CameraUBO, 0                    , sizeof(glm::mat4), glm::value_ptr(Camera.Perspective));
    glNamedBufferSubData(CameraUBO, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(Camera.Orthographic));
    glNamedBufferSubData(CameraUBO, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(Camera.View));
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
