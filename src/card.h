#pragma once

#include "typedefs.h"
#include "animation.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// World units are centimetres, card.hpp is a real 63 x 88 mm card so it's drawn without scaling
constexpr glm::vec2 CardSize = glm::vec2(6.3f, 8.8f);   // Centimeters
constexpr f32 CardThickness = 0.03f;                    // Centimeters

constexpr f32 CardMoveDuration = 0.35f; // Seconds
constexpr f32 CardDealDelay    = 0.08f; // Seconds, so the cards of a deal leave the Dungeon one by one
constexpr f32 CardMaxLift      = 4.0f;  // Centimeters, so a moving card passes over the piles

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

enum card_pile
{
    Pile_None,
    Pile_Dungeon,
    Pile_Room,
    Pile_DiscardPile,
    Pile_EquippedWeapon,
    Pile_SlainMonsters,
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
    card_pile Pile;

    i32 Value;          // 2-10, J = 11, Q = 12, K = 13, A = 14. Damage for monsters, strength for weapons, health for potions

    animation_state Animation;

    void Update(f32 DeltaTime);

    glm::mat3 RotationMatrix();
    b32 ContainsPoint(glm::vec3 Point);
};

card CreateCard(u64 Texture, u64 BackTexture, glm::vec3 Position, f32 Angle, glm::vec3 Axis, glm::vec4 Tint, card_pile Pile);
