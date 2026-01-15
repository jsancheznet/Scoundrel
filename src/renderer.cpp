#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

    // Configure Blend Mode
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glCreateVertexArrays(1, &MainVAO);
    glBindVertexArray(MainVAO);

    CurrentShader = 0;

    // TODO(Jsanchez): Delete this
    // vec3 CameraPos = vec3(0.0f, 0.0f, 1.0f);
    // vec3 CameraTarget = vec3(0.0f);
    // vec3 CameraUp = vec3(0.0f, 1.0f, 0.0f);
    // View = glm::lookAt(CameraPos, CameraTarget, CameraUp);
    // f32 Fov = 90.0f;
    // f32 CameraNear = 0.01f;
    // f32 CameraFar = 1000.0f;
    // f32 AspectRatio = (f32) ViewportWidth / (f32)ViewportHeight;
    // Projection = glm::perspective(glm::radians(Fov), AspectRatio, CameraNear, CameraFar);

    { // Upload Quad Data

        float Vertices[] =
        {
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, // top left
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
            0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f  // bottom right
        };


        // Create and allocate Buffer Storage
        glCreateBuffers(1, &QuadVBO);
        glNamedBufferStorage(QuadVBO, sizeof(Vertices), Vertices, GL_DYNAMIC_STORAGE_BIT);

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

texture renderer::CreateTexture(const char *File)
{
    texture Result;

    i32 ChannelCount;

    stbi_set_flip_vertically_on_load(true);

    u8 *ImageData = stbi_load(File, &Result.Width, &Result.Height, &ChannelCount, 4);

    glCreateTextures(GL_TEXTURE_2D, 1, &Result.Id);

    glTextureParameteri(Result.Id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Result.Id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Result.Id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(Result.Id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(Result.Id, 1, GL_RGBA8, Result.Width, Result.Height);

    glTextureSubImage2D(Result.Id, 0, 0, 0, Result.Width, Result.Height, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

    glGenerateTextureMipmap(Result.Id);

    stbi_image_free(ImageData);

    return Result;
}

void renderer::DrawTexture(texture Texture, vec3 Position, f32 Rotation, f32 Scale)
{
    u32 ModelMatrixUniformId = glGetUniformLocation(CurrentShader, "Model");

    mat4 Model = glm::mat4(1.0f);

    Model = glm::scale(Model, glm::vec3(Scale));
    vec3 RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    Model = glm::rotate(Model, Rotation, RotationAxis);
    Model = glm::translate(Model, Position);

    glUniformMatrix4fv(ModelMatrixUniformId, 1, GL_FALSE, value_ptr(Model));

    // Texture stuff
    glBindTextureUnit(0, Texture.Id);

    glUniform1i(glGetUniformLocation(CurrentShader, "Texture"), 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void renderer::UseShader(u32 Shader)
{
    CurrentShader = Shader;
    glUseProgram(Shader);
}

void renderer::UpdateCameraUniforms(camera Camera)
{
    u32 ViewId = glGetUniformLocation(CurrentShader, "View");
    glUniformMatrix4fv(ViewId, 1, GL_FALSE, value_ptr(Camera.View));

    u32 ProjectionId = glGetUniformLocation(CurrentShader, "Projection");
    glUniformMatrix4fv(ProjectionId, 1, GL_FALSE, value_ptr(Camera.Projection));
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
