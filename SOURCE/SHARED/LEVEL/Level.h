/**
 * Made by Ryan Devens on 2026-03-06
 */

#pragma once
#include <string>
#include <vector>
#include "LevelScene.h"

/**
 * Represents a single game level loaded from a LEVELS subdirectory.
 * Data is sourced from the level's level_info.json and its SCENE_N subdirectories.
 */
struct Level
{
    std::string             id;                 // directory name, e.g. "LEVEL_1"
    std::string             name;               // display name, e.g. "Level 1"
    bool                    isUnlocked = true;
    std::vector<LevelScene> scenes;
};
