#include "animation.h"

#include <glm/gtc/constants.hpp>

void animation_state::Animate(glm::vec3 FromPosition, f32 FromAngle, glm::vec3 ToPosition, f32 ToAngle, f32 StartDelay)
{
    f32 Distance = glm::length(glm::vec2(ToPosition - FromPosition)); // On the table plane

    IsActive = true;
    Delay = StartDelay;
    Elapsed = 0.0f;
    StartPosition = FromPosition;
    TargetPosition = ToPosition;
    StartAngle = FromAngle;
    TargetAngle = ToAngle;
    Lift = glm::min(MaxLift, Distance * 0.5f); // Short moves stay close to the table

    // Only the height changes (a pile was stacked again), there is nothing to see.
    // The next Update() gives the target values at once
    b32 SamePlace = Distance < 0.001f && glm::abs(ToAngle - FromAngle) < 0.001f;
    if(SamePlace)
    {
        Delay = 0.0f;
        Elapsed = Duration;
    }
}

// NewPosition and NewAngle are not written when there is no move or the delay is not over
void animation_state::Update(f32 DeltaTime, glm::vec3 *NewPosition, f32 *NewAngle)
{
    if(!IsActive) return;

    if(Delay > 0.0f)
    {
        Delay -= DeltaTime;
        if(Delay > 0.0f) return;

        // The time left of this tick after the delay ended goes to the move
        DeltaTime = -Delay;
        Delay = 0.0f;
    }

    Elapsed += DeltaTime;

    // NOTE: A Duration of 0 ends the move on the first update, there is no division by zero
    f32 T = (Duration > 0.0f) ? glm::clamp(Elapsed / Duration, 0.0f, 1.0f) : 1.0f;

    if(T >= 1.0f)
    {
        // Exactly on the targets, so there is no rounding error left
        *NewPosition = TargetPosition;
        *NewAngle = TargetAngle;
        IsActive = false;
        return;
    }

    f32 Eased = T * T * (3.0f - 2.0f * T);

    *NewPosition = glm::mix(StartPosition, TargetPosition, Eased);
    NewPosition->z += Lift * glm::sin(glm::pi<f32>() * T);
    *NewAngle = glm::mix(StartAngle, TargetAngle, Eased);
}
