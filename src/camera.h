#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include "typedefs.h"

// Forward declarations, renderer.h includes camera.h
struct renderer;
struct mouse;

struct camera
{
    vec3 Position;
    vec3 Target;
    vec3 Up;

    f32 Fov;
    f32 Near;
    f32 Far;
    f32 AspectRatio;

    mat4 View;
    mat4 Perspective;
    mat4 Orthographic;

    void Update(); // Recomputes the matrices and uploads them to the GPU, call it every frame after moving the camera
};


camera CreateCamera();

vec3 ScreenToWorldRay(mouse *Mouse, renderer *Renderer, camera *Camera);
b32 RayPlaneIntersection(vec3 RayOrigin, vec3 RayDirection, vec3 PlanePoint, vec3 PlaneNormal, vec3 *Hit);
