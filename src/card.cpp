#include "card.h"

card CreateCard(u64 Texture, u64 BackTexture, glm::vec3 Position, f32 Angle, glm::vec3 Axis, glm::vec4 Tint)
{
    card Result = {};

    Result.Texture = Texture;
    Result.BackTexture = BackTexture;
    Result.Position = Position;
    Result.Angle = Angle;
    Result.Axis = Axis;
    Result.Scale = CardSize;
    Result.Tint = Tint;

    return Result;
}
