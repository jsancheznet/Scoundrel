#include <stdio.h>

#include "renderer.h"

void renderer::Init(SDL_Window* SDLWindow, u32 Width, u32 Height)
{
    Window = SDLWindow;
    ViewportWidth = Width;
    ViewportHeight = Height;

    gladLoadGL();

    // Enable Debug Mode
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);

    glViewport(0, 0, ViewportWidth, ViewportHeight);

    // TODO(Jsanchez): Configure blend mode
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glCreateVertexArrays(1, &MainVAO);
    glBindVertexArray(MainVAO);

    CurrentShader = 0;

    vec3 CameraPos = vec3(0.0f, 0.0f, -1.0f);
    vec3 CameraTarget = vec3(0.0f);
    vec3 CameraUp = vec3(0.0f, 1.0f, 0.0f);
    View = glm::lookAt(CameraPos, CameraTarget, CameraUp);

    f32 Fov = 90.0f;
    f32 CameraNear = 0.01f;
    f32 CameraFar = 1000.0f;
    f32 AspectRatio = (f32) ViewportWidth / (f32)ViewportHeight;
    Projection = glm::perspective(glm::radians(Fov), AspectRatio, CameraNear, CameraFar);
}

void renderer::ClearScreen(color Color)
{
    glClearColor(Color.r, Color.g, Color.b, Color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer::EndFrame()
{
    SDL_GL_SwapWindow(Window);
}

u32 renderer::CompileShader(const char *Filename)
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

triangle renderer::CreateTriangle()
{
    triangle Result;

    f32 Vertices[] =
    {
        0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    Result.Position = glm::vec3(0.0f);
    Result.Scale = glm::vec3(1.0f);
    Result.Rotation = 0.0f;

    u32 Buffer;
    u32 BindingIndex = 0;
    u32 Offset = 0;
    u32 VertexSize = 3; // 3 floats
    u32 AttributeIndex = 0;

    glCreateBuffers(1, &Buffer);

    // Allocates buffer storage
    glNamedBufferStorage(Buffer, sizeof(Vertices), Vertices, GL_DYNAMIC_STORAGE_BIT);

    // Binds a buffer to a vertex binding point
    glVertexArrayVertexBuffer(MainVAO, BindingIndex, Buffer, Offset, sizeof(f32) * 3);

    // Enable the vertex attribute array on Binding Index
    glEnableVertexArrayAttrib(MainVAO, BindingIndex);

    // Specify the organization of vertex arrays
    glVertexArrayAttribFormat(MainVAO, AttributeIndex, VertexSize, GL_FLOAT, GL_FALSE, Offset);

    glVertexArrayAttribBinding(MainVAO, AttributeIndex, BindingIndex);

    Result.Id = Buffer;

    return Result;
}

void renderer::DrawTriangle(triangle Triangle)
{
    // TODO(Jsanchez): Create something more performant that doing 1 draw call per object

    u32 UniformModelId = glGetUniformLocation(CurrentShader, "Model");

    mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, Triangle.Scale);
    vec3 RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    Model = glm::rotate(Model, Triangle.Rotation, RotationAxis);
    Model = glm::translate(Model, Triangle.Position);

    glUniformMatrix4fv(UniformModelId, 1, GL_FALSE, value_ptr(Model));

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer::UseShader(u32 Shader)
{
    CurrentShader = Shader;
    glUseProgram(Shader);
}

void renderer::UpdateCameraUniforms()
{
    u32 ViewId = glGetUniformLocation(CurrentShader, "View");
    glUniformMatrix4fv(ViewId, 1, GL_FALSE, value_ptr(View));

    u32 ProjectionId = glGetUniformLocation(CurrentShader, "Projection");
    glUniformMatrix4fv(ProjectionId, 1, GL_FALSE, value_ptr(Projection));
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

    printf("%d: %s of %s severity, raised from %s: %s\n", Id, _Type, _Severity, _Source, Message);
}
