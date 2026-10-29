#include "typedefs.h"
#include "card.h"

card CreateCard(u64 Texture, u64 BackTexture, glm::vec3 Position, f32 Angle, glm::vec3 Axis, glm::vec4 Tint, card_pile Pile)
{
    card Result = {};

    Result.Texture = Texture;
    Result.BackTexture = BackTexture;
    Result.Position = Position;
    Result.Angle = Angle;
    Result.Axis = Axis;
    Result.Scale = CardSize;
    Result.Tint = Tint;
    Result.Pile = Pile;

    return Result;
}

glm::mat3 card::RotationMatrix()
{
    return glm::mat3(glm::rotate(glm::mat4(1.0f), Angle, Axis));
}

b32 card::ContainsPoint(glm::vec3 Point)
{
    // Point must be on the card plane, returns true if it lands inside the card rectangle
    // The card's own width and height directions in world space
    glm::mat3 Rotation = RotationMatrix();
    glm::vec3 Right = Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 Up    = Rotation * glm::vec3(0.0f, 1.0f, 0.0f);

    // How far the point is from the card center along the card's width and height
    // NOTE: Right and Up are unit length (pure rotation), so the dot products are distances
    glm::vec3 Offset = Point - Position;
    f32 X = glm::dot(Offset, Right);
    f32 Y = glm::dot(Offset, Up);

    glm::vec2 HalfSize = 0.5f * Scale;
    return glm::abs(X) <= HalfSize.x && glm::abs(Y) <= HalfSize.y;
}
