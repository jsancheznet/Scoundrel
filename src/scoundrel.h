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
constexpr u32 RoomSize = 4;
constexpr f32 SfxVolume = 0.4f;
constexpr f32 MusicVolume = SfxVolume * 0.5f; // The music plays at half the volume of the sound effects
constexpr u32 CardsToFacePerRoom = 3; // A turn ends once 3 of the 4 Room cards were faced, the 4th stays for the next Room
constexpr f32 SlainMonsterStagger = 1.8f; // cm between slain monsters on the weapon, wider than the number box of a card

// Screen text sizes are given for this window height, they scale with the window like the table does
constexpr f32 ReferenceWindowHeight = 768.0f;

// Floating Text Labels
constexpr f32 LabelGapAboveCard   = 2.0f;  // cm from the top edge of a card to the top of its label
constexpr f32 LabelFloatSpeed     = 2.0f;  // rad/s, higher is faster (2.0 is about one cycle every 3 s)
constexpr f32 LabelFloatHeight    = 0.08f; // cm above and below the rest position
constexpr f32 LabelShadowOffset   = 0.15f; // cm below the rest position of the label
constexpr f32 LabelShadowMaxScale = 1.02f; // Shadow scale when the label is at its highest point (1.0 at the lowest)
constexpr f32 LabelShadowFollow   = 0.5f;  // Part of the label motion the shadow follows (0 = still, 1 = moves with the label)
constexpr f32 LabelShadowAlphaLow  = 0.6f; // Shadow alpha when the label is at its lowest point (darkest)
constexpr f32 LabelShadowAlphaHigh = 0.45f; // Shadow alpha when the label is at its highest point (lightest)

enum game_state
{
    Game_WaitingToStart, // Dungeon is shuffled, waiting for the first click on it
    Game_Playing,
    Game_Won,            // Made it through the whole Dungeon
    Game_Lost,           // Health reached 0
};

struct game
{
    void Init(renderer *Renderer, audio_system *Audio, keyboard *Keyboard, mouse *Mouse, asset_manager *AssetMgr);
    void Update();
    void ProcessInput();
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

    void DrawHealth();
    void DrawCards();
    void DrawLabels();
    void UpdateActionHint();
    void DrawActionHint();

    card *PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection);

    camera Camera;

    u64 MarcellusFont;

    // Sound Effects
    u64 SlashSound;
    u64 PunchSound;
    u64 DamageSound;
    u64 HealthPotionSound;
    u64 ErrorSound;

    u64 Music;

    game_state State;
    i32 PlayerHealth;
    i32 Score;

    // Text Buffer - Used to tell the player what to do / what's possible
    char ActionHint[256];

    // Turn state
    u32 CardsFacedThisRoom;
    b32 PotionUsedThisRoom;
    b32 AvoidedLastRoom;
    b32 LastCardWasPotion;
    i32 LastPotionValue;

    // Pile Positions
    glm::vec3 DungeonPosition;
    glm::vec3 DiscardPosition;
    glm::vec3 WeaponPosition;

    // Room Card Positions
    glm::vec3 RoomCardPosition1;
    glm::vec3 RoomCardPosition2;
    glm::vec3 RoomCardPosition3;
    glm::vec3 RoomCardPosition4;

    // Card Piles
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
