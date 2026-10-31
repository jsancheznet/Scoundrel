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

    camera Camera;

    u64 MarcellusFont;

    u32 PlayerHealth;

    glm::vec3 DungeonPosition;
    std::vector<card> Dungeon;

private:
    renderer *Renderer;
    mouse *Mouse;
    keyboard *Keyboard;
    audio_system *Audio;
    asset_manager *AssetMgr;

    u64 CardBackTexture;
};
