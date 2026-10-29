#pragma once

#include "card.h"

std::vector<card> LoadDungeon(u64 BackTexture, glm::vec3 DungeonPosition);
card *PickCard(std::vector<card> &Cards, glm::vec3 RayOrigin, glm::vec3 RayDirection);
