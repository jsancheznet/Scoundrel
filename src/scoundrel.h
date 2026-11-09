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
constexpr u32 CardsToFacePerRoom = 3; // A turn ends once 3 of the 4 Room cards were faced, the 4th stays for the next Room

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
    void DrawStatus();

    card *PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection);

    camera Camera;

    u64 MarcellusFont;

    game_state State;
    i32 PlayerHealth;
    i32 Score;

    // Turn state
    u32 CardsFacedThisRoom;
    b32 PotionUsedThisTurn;     // Only one potion heals per turn, the second one is simply discarded
    b32 AvoidedLastRoom;        // Two Rooms in a row can't be avoided
    b32 LastCardWasPotion;      // For the score, a full health win adds the value of the last potion
    i32 LastPotionValue;

    glm::vec3 DungeonPosition;
    glm::vec3 DiscardPosition;
    glm::vec3 WeaponPosition;

    glm::vec3 Room1;
    glm::vec3 Room2;
    glm::vec3 Room3;
    glm::vec3 Room4;

    std::vector<card> Dungeon;          // The back of the vector is the top of the pile
    std::vector<card> Room;
    std::vector<card> Weapon;           // 0 or 1 cards, the equipped weapon
    std::vector<card> SlainMonsters;    // Monsters slain by the equipped weapon, the back is the last one
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
};
