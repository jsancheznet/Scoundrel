#pragma once

#include <vector>

#include "asset_manager.h"
#include "typedefs.h"
#include "renderer.h"
#include "mouse.h"
#include "keyboard.h"
#include "audio.h"
#include "camera.h"

constexpr i32 MaxHealth = 20;
constexpr u32 DeckSize = 44;
constexpr u32 RoomSize = 4;
constexpr f32 SfxVolume = 0.4f;
constexpr f32 MusicVolume = SfxVolume * 0.5f;
constexpr u32 CardsToFacePerRoom = 3; // A turn ends once 3 of the 4 Room cards were faced, the 4th stays for the next Room
constexpr f32 SlainMonsterStagger = 1.8f; // Centimeters, wider than the number box of a card
constexpr f32 MaxAnimationDeltaTime = 0.05f; // Seconds, longer frames are cut to this so the cards don't jump

// Screen text sizes are given for this window height, they scale with the window like the table does
constexpr f32 ReferenceWindowHeight = 768.0f;

// The camera is above the center of the table, so the table is lower on the screen and the action hint fits above the Room label
constexpr f32 CameraHeight = 1.4f; // Centimeters

constexpr f32 DamageShakeStrength = 0.45f; // Centimeters
constexpr f32 DamageShakeDuration = 0.25f; // Seconds

constexpr f32 WeaponHitDuration    = 0.3f; // Seconds
constexpr f32 WeaponReturnDuration = 0.2f; // Seconds

// With the perspective, a label on the table would not line up with a tall pile
constexpr f32 LabelHeight = CardThickness * (f32)(DeckSize - 1); // Centimeters

constexpr f32 LabelGapAboveCard   = 2.0f;  // Centimeters
constexpr f32 LabelFloatSpeed     = 2.0f;  // Radians per second
constexpr f32 LabelFloatHeight    = 0.08f; // Centimeters
constexpr f32 LabelShadowOffset   = 0.15f; // Centimeters
constexpr f32 LabelShadowMaxScale = 1.02f;
constexpr f32 LabelShadowFollow   = 0.5f;  // 0 = still, 1 = moves with the label
constexpr f32 LabelShadowAlphaLow  = 0.6f;
constexpr f32 LabelShadowAlphaHigh = 0.45f;

enum game_state
{
    Game_WaitingToStart,
    Game_Playing,
    Game_Won,
    Game_Lost,
};

struct game
{
    void Init(renderer *Renderer, audio_system *Audio, keyboard *Keyboard, mouse *Mouse, asset_manager *AssetMgr);
    void Update(f32 DeltaTime);
    void ProcessInput();
    void UpdateCards(f32 DeltaTime);
    void Render();

    void LoadDungeon();
    void NewGame();
    void DealRoom();
    void AvoidRoom();
    void FaceRoomCard(u32 Index, b32 Barehanded);
    void EquipWeapon(card Weapon);
    void FightMonster(card Monster, b32 Barehanded);
    void DrinkPotion(card Potion);
    void CheckEndOfTurn();

    static void OnWeaponHit(void *UserData);

    void DrawHealth();
    void DrawCards();
    void DrawLabels();
    void UpdateActionHint();
    void DrawActionHint();

    card *PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection);

    camera Camera;

    u64 MarcellusFont;

    u64 SlashSound;
    u64 PunchSound;
    u64 DamageSound;
    u64 HealthPotionSound;
    u64 ErrorSound;

    u64 Music;

    game_state State;
    i32 PlayerHealth;
    i32 Score;

    char ActionHint[256];

    u32 CardsFacedThisRoom;
    b32 PotionUsedThisRoom;
    b32 AvoidedLastRoom;
    b32 LastCardWasPotion;
    i32 LastPotionValue;
    b32 WeaponHitDamaged;

    glm::vec3 DungeonPosition;
    glm::vec3 DiscardPosition;
    glm::vec3 WeaponPosition;

    glm::vec3 RoomCardPosition1;
    glm::vec3 RoomCardPosition2;
    glm::vec3 RoomCardPosition3;
    glm::vec3 RoomCardPosition4;

    std::vector<card> Dungeon;
    std::vector<card> Room;
    std::vector<card> Weapon;
    std::vector<card> SlainMonsters;
    std::vector<card> DiscardPile;

private:
    renderer *Renderer;
    mouse *Mouse;
    keyboard *Keyboard;
    audio_system *Audio;
    asset_manager *AssetMgr;

    u64 CardBackTexture;

    void MoveCards(std::vector<card> &Src, std::vector<card> &Dst, u32 Count, card_pile DstPile);
    void Discard(card Card);
    void StackDungeon();
    void DrawFloatingLabel(glm::vec3 Center, const char *Text, f32 Wave);
    f32 ScreenTextScale();
};
