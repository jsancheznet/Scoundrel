#pragma once

#include "typedefs.h"

#include <glm/glm.hpp>

enum animation_type
{
    Animation_Move,
    Animation_Attack,
};

typedef void animation_callback(void *UserData);

// Only calculates the values, the owner applies them to its own object
struct animation_state
{
    // Set by the owner of the animation
    f32 Duration; // Seconds
    f32 MaxLift;  // Centimeters

    animation_type Type;
    b32 IsActive;
    f32 Delay;    // Seconds
    f32 Elapsed;  // Seconds

    glm::vec3 StartPosition;
    glm::vec3 TargetPosition;
    f32 StartAngle;  // Radians
    f32 TargetAngle;
    f32 Lift;        // Centimeters

    f32 HitDuration;    // Seconds
    f32 ReturnDuration; // Seconds
    b32 HitDone;
    animation_callback *OnHit; // Called one time, on the update where an attack gets to its hit position. Can be nullptr
    void *OnHitUserData;

    void Move(glm::vec3 FromPosition, f32 FromAngle, glm::vec3 ToPosition, f32 ToAngle, f32 StartDelay = 0.0f);
    void Attack(glm::vec3 FromPosition, glm::vec3 HitPosition, f32 HitDurationSeconds, f32 ReturnDurationSeconds,
                animation_callback *OnHitCallback = nullptr, void *UserData = nullptr);
    void Update(f32 DeltaTime, glm::vec3 *NewPosition, f32 *NewAngle);
};
