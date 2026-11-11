#pragma once

#include "typedefs.h"

#include <glm/glm.hpp>

// Only calculates the values, the owner applies them to its own object
struct animation_state
{
    // Set by the owner of the animation
    f32 Duration; // Seconds
    f32 MaxLift;  // Centimeters

    b32 IsActive;
    f32 Delay;    // Seconds
    f32 Elapsed;  // Seconds

    glm::vec3 StartPosition;
    glm::vec3 TargetPosition;
    f32 StartAngle;  // Radians
    f32 TargetAngle;
    f32 Lift;        // Centimeters

    void Animate(glm::vec3 FromPosition, f32 FromAngle, glm::vec3 ToPosition, f32 ToAngle, f32 StartDelay = 0.0f);
    void Update(f32 DeltaTime, glm::vec3 *NewPosition, f32 *NewAngle);
};
