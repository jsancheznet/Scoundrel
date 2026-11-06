#pragma once

#include <vector>

#include "asset_manager.h"
#include "typedefs.h"
#include "renderer.h"
#include "mouse.h"
#include "keyboard.h"
#include "audio.h"
#include "asset_manager.h"
#include "camera.h"

struct game
{
    void Init(renderer *Renderer, audio_system *Audio, keyboard *Keyboard, mouse *Mouse, asset_manager *AssetMgr);
    void Update();
    void Render();

    void LoadDungeon();

    void DrawHealth();
    void DrawCards();

    card *PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection);

    camera Camera;

    u64 MarcellusFont;

    u32 PlayerHealth;
    b32 GameStarted;

    glm::vec3 DungeonPosition;
    glm::vec3 DiscardPosition;
    glm::vec3 WeaponPosition;

    glm::vec3 Room1;
    glm::vec3 Room2;
    glm::vec3 Room3;
    glm::vec3 Room4;

    std::vector<card> Dungeon;
    std::vector<card> Room;

private:
    renderer *Renderer;
    mouse *Mouse;
    keyboard *Keyboard;
    audio_system *Audio;
    asset_manager *AssetMgr;

    u64 CardBackTexture;

    void MoveCards(std::vector<card> &Src, std::vector<card> &Dst, u32 Count);
};
