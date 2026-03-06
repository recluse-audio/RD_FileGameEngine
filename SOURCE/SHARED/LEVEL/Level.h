/**
 * Made by Ryan Devens on 2026-03-06
 */

#pragma once
#include <map>
#include <string>

/**
 * Represents a single game level loaded from a LEVELS subdirectory.
 * Data is sourced from the level's level_info.json file.
 */
struct Level
{
    std::string                        id;     // directory name, e.g. "LEVEL_1"
    std::string                        name;   // display name, e.g. "Level 1"
    std::map<std::string, std::string> assets; // virtual path -> friendly name
};
