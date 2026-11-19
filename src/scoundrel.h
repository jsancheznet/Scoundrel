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
constexpr f32 ScreenTextMargin    = 16.0f; // Pixels
constexpr f32 ActionHintSize      = 28.0f; // Pixels

constexpr const char *ActionHintCards = "Left click: use card   Right click: fight barehanded";
constexpr const char *ActionHintSkip  = "   Click Dungeon: skip Room";
constexpr f32 ActionHintUnavailableAlpha = 0.35f;
constexpr f32 OverlayTitleSize    = 64.0f; // Pixels
constexpr f32 OverlaySubtitleSize = 30.0f; // Pixels

constexpr f32 OverlayDarkness = 0.45f; // 1 = black
constexpr f32 OverlayFadeDuration = 0.3f; // Seconds

// The camera is above the center of the table, so the table is lower on the screen and the action hint fits above the Room label
constexpr f32 CameraHeight = 1.4f; // Centimeters

constexpr f32 DamageShakeStrength = 0.45f; // Centimeters
constexpr f32 DamageShakeDuration = 0.25f; // Seconds

constexpr color DamageFlashColor = {0.55f, 0.04f, 0.05f, 1.0f};
constexpr color HealFlashColor   = {0.08f, 0.50f, 0.15f, 1.0f};
constexpr f32 BackgroundFlashMaxTint = 0.75f; // 1 = the background is fully the flash color
constexpr f32 BackgroundFlashRiseDuration = 0.08f; // Seconds
constexpr f32 BackgroundFlashFallDuration = 0.6f;  // Seconds
constexpr f32 BackgroundFlashDuration = BackgroundFlashRiseDuration + BackgroundFlashFallDuration;

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

constexpr f32 HealthTextHeight = 1.7f; // Centimeters
constexpr f32 HealthIconSize   = 2.0f; // Centimeters
constexpr f32 HealthIconGap    = 0.4f; // Centimeters, between the heart and the number

constexpr u32 CardPlaceSoundCount = 4;
constexpr f32 RoomDealPause = 0.0625f; // Seconds, after the last card lands and before the next Room is dealt

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
    void UpdateBackgroundFlash(f32 DeltaTime);
    void UpdateOverlay(f32 DeltaTime);
    b32 OverlayIsWanted();
    void Render();

    void LoadDungeon();
    void NewGame();
    void StartPlaying();
    void DealRoom();
    void AvoidRoom();
    void FaceRoomCard(u32 Index, b32 Barehanded);
    void EquipWeapon(card Weapon);
    void FightMonster(card Monster, b32 Barehanded);
    void DrinkPotion(card Potion);
    void CheckEndOfTurn();
    void OnPlayerDamaged();
    void OnPlayerHealed();
    void FlashBackground(color Color);

    static void OnWeaponHit(void *UserData, u64 Data);
    static void OnCardDealt(void *UserData, u64 Data);

    void DrawHealth();
    void DrawCards();
    void DrawLabels();
    void UpdateStateText();
    void DrawActionHint();
    void DrawOverlayText();
    void DrawCenteredScreenText(const char *Text, f32 Size, f32 Y, color Color);

    card *PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection);

    camera Camera;

    u64 MarcellusFont;

    u64 SlashSound;
    u64 PunchSound;
    u64 DamageSound;
    u64 HealthPotionSound;
    u64 ErrorSound;
    u64 CardPlaceSounds[CardPlaceSoundCount]; // The order changes on each deal

    u64 Music;

    game_state State;
    i32 PlayerHealth;
    i32 Score;

    b32 CanAvoidRoom;
    char OverlayTitle[64];
    char OverlaySubtitle[128];
    f32 OverlayAmount; // 0 = no overlay, 1 = full, follows the fade
    b32 CardsAreMoving;
    b32 RoomDealPending; // The next Room is dealt once all cards have landed and RoomDealPause has passed
    f32 RoomDealWait;    // Seconds

    u32 CardsFacedThisRoom;
    b32 PotionUsedThisRoom;
    b32 AvoidedLastRoom;
    b32 LastCardWasPotion;
    i32 LastPotionValue;
    b32 WeaponHitDamaged;
    effect_timer BackgroundFlashTimer;
    color BackgroundFlashColor;
    b32 BackgroundFlashHeld; // The killing hit keeps the red until a new game

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
    u64 HeartTexture;
    b32 HasHeartTexture;

    void MoveCards(std::vector<card> &Src, std::vector<card> &Dst, u32 Count, card_pile DstPile);
    void Discard(card Card);
    void StackDungeon();
    void DrawFloatingLabel(glm::vec3 Center, const char *Text, f32 Wave);
    f32 ScreenTextScale();
    f32 FitCenteredScreenText(bitmap_font *Font, const char *Text, f32 *Size); // Returns the left X in pixels, makes Size smaller if the text is wider than the window
};
