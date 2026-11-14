#include "animation.h"

#include <glm/gtc/constants.hpp>

// NOTE: The value goes below 0 at the start, so the object pulls back before it goes forward
static f32 EaseInBack(f32 T)
{
    f32 C1 = 1.70158f;
    f32 C3 = C1 + 1.0f;

    return C3 * T * T * T - C1 * T * T;
}

void animation_state::Move(glm::vec3 FromPosition, f32 FromAngle, glm::vec3 ToPosition, f32 ToAngle, f32 StartDelay)
{
    f32 Distance = glm::length(glm::vec2(ToPosition - FromPosition)); // On the table plane

    Type = Animation_Move;
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

void animation_state::Attack(glm::vec3 FromPosition, glm::vec3 HitPosition, f32 HitDurationSeconds, f32 ReturnDurationSeconds,
                             animation_callback *OnHitCallback, void *UserData)
{
    f32 Distance = glm::length(glm::vec2(HitPosition - FromPosition)); // On the table plane

    Type = Animation_Attack;
    IsActive = true;
    Delay = 0.0f;
    Elapsed = 0.0f;
    StartPosition = FromPosition;
    TargetPosition = HitPosition;
    HitDuration = HitDurationSeconds;
    ReturnDuration = ReturnDurationSeconds;
    HitDone = false;
    OnHit = OnHitCallback;
    OnHitUserData = UserData;
    Lift = glm::min(MaxLift, Distance * 0.5f);
}

// NewPosition and NewAngle are not written when there is no move or the delay is not over.
// An attack does not write NewAngle
void animation_state::Update(f32 DeltaTime, glm::vec3 *NewPosition, f32 *NewAngle)
{
    if(!IsActive)
    {
        return;
    }

    if(Delay > 0.0f)
    {
        Delay -= DeltaTime;
        if(Delay > 0.0f)
        {
            return;
        }

        // The time left of this tick after the delay ended goes to the move
        DeltaTime = -Delay;
        Delay = 0.0f;
    }

    Elapsed += DeltaTime;

    switch(Type)
    {
        case Animation_Move:
        {
            // NOTE: A Duration of 0 ends the move on the first update, there is no division by zero
            f32 T = (Duration > 0.0f) ? glm::clamp(Elapsed / Duration, 0.0f, 1.0f) : 1.0f;

            if(T >= 1.0f)
            {
                // Exactly on the targets, so there is no rounding error left
                *NewPosition = TargetPosition;
                *NewAngle = TargetAngle;
                IsActive = false;
            }
            else
            {
                f32 Eased = T * T * (3.0f - 2.0f * T);

                *NewPosition = glm::mix(StartPosition, TargetPosition, Eased);
                NewPosition->z += Lift * glm::sin(glm::pi<f32>() * T);
                *NewAngle = glm::mix(StartAngle, TargetAngle, Eased);
            }
        } break;
        case Animation_Attack:
        {
            if(Elapsed < HitDuration)
            {
                f32 T = Elapsed / HitDuration;

                *NewPosition = glm::mix(StartPosition, TargetPosition, EaseInBack(T));
                // The linear T, the eased value is negative at the start and would push the object below the table
                NewPosition->z += Lift * glm::sin(glm::pi<f32>() * T);
            }
            else
            {
                if(!HitDone)
                {
                    HitDone = true;
                    if(OnHit)
                    {
                        OnHit(OnHitUserData);
                    }
                }

                f32 ReturnElapsed = Elapsed - HitDuration;
                f32 T = (ReturnDuration > 0.0f) ? glm::clamp(ReturnElapsed / ReturnDuration, 0.0f, 1.0f) : 1.0f;

                if(T >= 1.0f)
                {
                    *NewPosition = StartPosition;
                    IsActive = false;
                }
                else
                {
                    *NewPosition = glm::mix(TargetPosition, StartPosition, T);
                    NewPosition->z += Lift * glm::sin(glm::pi<f32>() * T);
                }
            }
        } break;
    }
}
