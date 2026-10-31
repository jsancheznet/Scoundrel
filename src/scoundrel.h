#pragma once

#include <vector>

#include "card.h"

std::vector<card> LoadDungeon(glm::vec3 DungeonPosition);

card *PickCard(std::vector<card> &Cards, glm::vec3 RayOrigin, glm::vec3 RayDirection);

void DrawCards(const std::vector<card> &Cards);
void DrawHealth(u32 Health, u64 Font);
