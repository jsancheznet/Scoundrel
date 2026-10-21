#pragma once

#include "typedefs.h"

// NOTE: The state lives inside an inline function, so every .cpp that includes this header shares the same one.
// A static global would give each .cpp its own copy, and seeding in one file wouldn't affect the others.
inline
u32 *RandomState()
{
    static u32 State = 1;
    return &State;
}

inline
void RandomSeed(u32 Input)
{
    Assert(Input != 0);

    *RandomState() = Input;
}

inline
u32 RandomU32()
{
    u32 X = *RandomState();
    X ^= X << 13;
    X ^= X >> 17;
    X ^= X << 5;
    return *RandomState() = X;
}

inline
i32 RandomI32()
{
    return (i32)RandomU32();
}

// Returns a value in [Min, Max), Max is never returned
inline
i32 RandomBetween(i32 Min, i32 Max)
{
    Assert(Min < Max);

    // Scale the 32 bit random number into the range instead of dividing, so it can never reach Max
    u32 Range = (u32)((i64)Max - (i64)Min);
    return Min + (i32)(((u64)RandomU32() * Range) >> 32);
}

// Returns a value in [Min, Max), Max is never returned
inline
f32 RandomBetween(f32 Min, f32 Max)
{
    Assert(Min < Max);

    // Use the top 24 bits, a f32 represents all of them exactly, so Normalized stays below 1.0
    f32 Normalized = (f32)(RandomU32() >> 8) * (1.0f / 16777216.0f);
    return Min + (Max - Min) * Normalized;
}

inline
b32 RandomBool()
{
    return RandomU32() % 2;
}
