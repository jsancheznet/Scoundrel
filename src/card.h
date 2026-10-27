#pragma once

#include "typedefs.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

enum card_suit
{
    Suit_Clubs,
    Suit_Spades,
    Suit_Diamonds,
    Suit_Hearts,

    Suit_Count
};

enum card_type
{
    Card_Monster, // Clubs and Spades
    Card_Weapon,  // Diamonds
    Card_Potion,  // Hearts
};

struct card
{
    u64 Texture;
    u64 BackTexture;
    glm::vec3 Position;
    f32 Angle;
    glm::vec3 Axis;
    glm::vec2 Scale;
    glm::vec4 Tint;

    card_suit Suit;
    card_type Type;
    i32 Value;          // 2-10, J = 11, Q = 12, K = 13, A = 14. Damage for monsters, strength for weapons, health for potions
};

constexpr f32 CardMeshScale = 0.13f; // The card mesh is 6.3 x 8.8 units, this scales it down to the table size
constexpr glm::vec2 CardSize = glm::vec2(6.3f * CardMeshScale, 8.8f * CardMeshScale);
constexpr f32 CardThickness = 0.03f * CardMeshScale; // The card mesh is 0.03 units thick

card CreateCard(u64 Texture, u64 BackTexture, glm::vec3 Position, f32 Angle, glm::vec3 Axis, glm::vec4 Tint);
