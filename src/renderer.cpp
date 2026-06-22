#include <stdio.h>

#include "renderer.h"
#include "log.h"

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

    glCreateVertexArrays(1, &MainVAO);
    glBindVertexArray(MainVAO);

    CurrentShader = 0;

    SpriteList.reserve(MAX_SPRITE_COUNT);

    { // Create mesh that holds the card

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
        glVertexArrayVertexBuffer(MainVAO, BindingPoint, QuadVBO, 0, sizeof(f32) * 5); // 5 floats

        // Vertex Attribute - Configure Vertex Attribute 0 (Position) from the interleaved buffer data
        glEnableVertexArrayAttrib(MainVAO, 0);
        glVertexArrayAttribFormat(MainVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(MainVAO, 0, 0);

        // UV Attribute - Configure Vertex Attribute 1 (UV) from the interleaved buffer data
        glEnableVertexArrayAttrib(MainVAO, 1);
        glVertexArrayAttribFormat(MainVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 3);
        glVertexArrayAttribBinding(MainVAO, 1, 0);
    }

    { // SpritesVBO

        glCreateBuffers(1, &SpritesVBO);
        u32 BufferSize = sizeof(sprite_instance) * MAX_SPRITE_COUNT;
        glNamedBufferStorage(SpritesVBO, BufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
        Log(Info, "OPENGL, Allocating %d bytes to SpritesVBO", BufferSize);

        u32 BindingPoint = 3;
        glVertexArrayVertexBuffer(MainVAO, BindingPoint, SpritesVBO, 0, sizeof(sprite_instance));

        // Position
        glEnableVertexArrayAttrib(MainVAO, 2);
        glVertexArrayAttribFormat(MainVAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(MainVAO, 2, BindingPoint);

        // Scale
        glEnableVertexArrayAttrib(MainVAO, 3);
        glVertexArrayAttribFormat(MainVAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Scale));
        glVertexArrayAttribBinding(MainVAO, 3, BindingPoint);

        // Rotation
        glEnableVertexArrayAttrib(MainVAO, 4);
        glVertexArrayAttribFormat(MainVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, Rotation));
        glVertexArrayAttribBinding(MainVAO, 4, BindingPoint);

        // Texture Handle
        glEnableVertexArrayAttrib(MainVAO, 5);
        glVertexArrayAttribIFormat(MainVAO, 5, 2, GL_UNSIGNED_INT, offsetof(sprite_instance, TextureHandle));
        glVertexArrayAttribBinding(MainVAO, 5, BindingPoint);

        // Src Rect
        glEnableVertexArrayAttrib(MainVAO, 6);
        glVertexArrayAttribFormat(MainVAO, 6, 4, GL_FLOAT, GL_FALSE, offsetof(sprite_instance, SrcRect));
        glVertexArrayAttribBinding(MainVAO, 6, BindingPoint);

        glVertexArrayBindingDivisor(MainVAO, 3, 1);
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
    glNamedBufferSubData(SpritesVBO, 0, SpriteList.size() * sizeof(sprite_instance), &SpriteList[0]);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, SpriteList.size());

    SpriteList.clear();
    SDL_GL_SwapWindow(Window);
}

shader renderer::CompileShader(const char *Filename)
{
    Assert(Filename);

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

void renderer::DrawTexture(texture Texture, vec3 Position, f32 Scale, f32 Rotation, rect SrcRect)
{
    sprite_instance Sprite = {};

    Sprite.Position = Position;
    Sprite.Scale = glm::vec3(Scale);
    Sprite.Rotation = Rotation;
    Sprite.TextureHandle = Texture.Handle;
    Sprite.SrcRect = SrcRect;

    SpriteList.push_back(Sprite);
}

void renderer::UseShader(u32 Shader)
{
    CurrentShader = Shader;
    glUseProgram(Shader);
    glUniform1i(glGetUniformLocation(CurrentShader, "Texture"), 0);
}

void renderer::UpdateCamera(camera Camera)
{
    Camera.View = glm::lookAt(Camera.Position, Camera.Target, Camera.Up);
    Camera.Projection = glm::perspective(glm::radians(Camera.Fov), Camera.AspectRatio, Camera.Near, Camera.Far);

    glNamedBufferSubData(CameraUBO, 0                    , sizeof(glm::mat4), glm::value_ptr(Camera.Projection));
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
