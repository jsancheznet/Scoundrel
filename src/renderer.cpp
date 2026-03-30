#include <stdio.h>

#include "renderer.h"
#include "log.h"

void renderer::Init(SDL_Window* SDLWindow, u32 Width, u32 Height)
{
    Window = SDLWindow;
    ViewportWidth = Width;
    ViewportHeight = Height;

    gladLoadGL();

    // Log OpenGL info
    i32 MaxUniformBufferBindings = -1;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MaxUniformBufferBindings);
    Log(Info, "OpenGL Max Uniform Buffer Bindings: %d", MaxUniformBufferBindings);

    // Enable Debug Mode
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);

    glViewport(0, 0, ViewportWidth, ViewportHeight);

    // Configure Blend Mode
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glCreateVertexArrays(1, &MainVAO);
    glBindVertexArray(MainVAO);

    CurrentShader = 0;

    { // Create the quad mesh

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

    { // CardsVBO

        glCreateBuffers(1, &CardsVBO);
        u32 BufferSize = sizeof(instance_data) * 10000;
        glNamedBufferStorage(CardsVBO, BufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
        Log(Info, "OPENGL, Allocating %d bytes to CardsVBO", BufferSize);

        u32 BindingPoint = 3;
        glVertexArrayVertexBuffer(MainVAO, BindingPoint, CardsVBO, 0, sizeof(instance_data));

        // Position
        glEnableVertexArrayAttrib(MainVAO, 2);
        glVertexArrayAttribFormat(MainVAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(MainVAO, 2, BindingPoint);

        // Scale
        glEnableVertexArrayAttrib(MainVAO, 3);
        glVertexArrayAttribFormat(MainVAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(instance_data, Scale));
        glVertexArrayAttribBinding(MainVAO, 3, BindingPoint);

        // Rotation
        glEnableVertexArrayAttrib(MainVAO, 4);
        glVertexArrayAttribFormat(MainVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(instance_data, Rotation));
        glVertexArrayAttribBinding(MainVAO, 4, BindingPoint);

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
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer::EndFrame()
{
    DrawSpriteCount = 0;
    SDL_GL_SwapWindow(Window);
}

u32 renderer::CompileShader(const char *Filename) //
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

void renderer::DrawTexture(texture Texture, vec3 Position, f32 Scale, f32 Rotation)
{
    // Texture stuff
    glBindTextureUnit(0, Texture.Id); // TODO(Jsanchez): Do i need to bind the texture here?
    glUniform1i(glGetUniformLocation(CurrentShader, "Texture"), 0);

    {
        instance_data SpriteInstanceData = {};

        SpriteInstanceData.Position = Position;
        SpriteInstanceData.Scale = glm::vec3(Scale);
        SpriteInstanceData.Rotation = Rotation;

        u32 Offset = sizeof(instance_data) * DrawSpriteCount;

        glNamedBufferSubData(CardsVBO, Offset, sizeof(instance_data), &SpriteInstanceData);

        DrawSpriteCount++;
    }


    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, DrawSpriteCount);
}

void renderer::DrawTextureSlow(texture Texture, vec3 Position, f32 Scale, f32 Rotation)
{
    // Create Model Matrix
    i32 ModelMatrixUniformId = glGetUniformLocation(CurrentShader, "Model");
    GLenum Error = glGetError();

    mat4 Model = glm::mat4(1.0f);
    vec3 RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    Model = glm::translate(Model, Position);
    Model = glm::rotate(Model, glm::radians(Rotation), RotationAxis);
    Model = glm::scale(Model, glm::vec3(Scale));

    glUniformMatrix4fv(ModelMatrixUniformId, 1, GL_FALSE, value_ptr(Model));

    // Texture stuff
    glBindTextureUnit(0, Texture.Id); // TODO(Jsanchez): Do i need to bind the texture everytime i call draw? Maybe i could do it once when using shader
    glUniform1i(glGetUniformLocation(CurrentShader, "Texture"), 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void renderer::UseShader(u32 Shader)
{
    CurrentShader = Shader;
    glUseProgram(Shader);
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

    printf("[OPENGL DEBUG]: %d: %s of %s severity, raised from %s: %s\n", Id, _Type, _Severity, _Source, Message);
}
