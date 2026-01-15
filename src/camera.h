#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include "typedefs.h"

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
    mat4 Projection;
};


camera CreateCamera();
