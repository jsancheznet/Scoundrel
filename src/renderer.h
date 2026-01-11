#pragma once

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "typedefs.h"

#define HOT_PINK color{1.0f, 0.0f, 1.0f, 0.0f}
#define ORANGE   color{1.0f, 0.647f, 0.0f, 0.0f}

using namespace glm;

struct color
{
    f32 r, g, b, a;
};

struct triangle
{
    vec3 Position;
    vec3 Scale;
    f32 Rotation;

    u32 Id;
};

struct renderer
{
    SDL_Window* Window;
    u32 ViewportWidth;
    u32 ViewportHeight;

    u32 MainVAO;

    u32 CurrentShader;

    // TODO(Jsanchez): Pull these out once we have a proper camera
    mat4 View;
    mat4 Projection;

    void Init(SDL_Window *SDLWindow, u32 Width, u32 Height);

    u32 CompileShader(const char *Filename);

    void UpdateCameraUniforms();

    triangle CreateTriangle();
    void DrawTriangle(triangle Triangle);

    void UseShader(u32 Shader);
    void ClearScreen(color Color);
    void EndFrame();


private:

    static void DebugCallback(GLenum Source, GLenum Type, GLuint Id,  GLenum Severity, GLsizei Length, GLchar const *Message, void const *UserParam);
};
