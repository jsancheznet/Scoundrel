#pragma once

#include <glm/glm.hpp>

#include "typedefs.h"

// A zero-initialized timer is not active, so an effect does not play at startup
struct effect_timer
{
    f32 Duration; // Seconds
    f32 Elapsed;  // Seconds

    void Start(f32 NewDuration, f32 StartElapsed = 0.0f) // Seconds
    {
        Duration = NewDuration;
        Elapsed = StartElapsed;
    }

    b32 IsActive() const
    {
        return Elapsed < Duration;
    }

    // Elapsed stops at Duration, so a long frame cannot push the effect past its end values
    void Update(f32 DeltaTime)
    {
        if(IsActive())
        {
            Elapsed = glm::min(Elapsed + DeltaTime, Duration);
        }
    }

    // 0 at the start, 1 at the end
    f32 Progress() const
    {
        if(Duration <= 0.0f)
        {
            return 1.0f;
        }

        return Elapsed / Duration;
    }
};
