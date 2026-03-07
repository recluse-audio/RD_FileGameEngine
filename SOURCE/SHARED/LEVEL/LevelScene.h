/**
 * Made by Ryan Devens on 2026-03-07
 */

#pragma once
#include <string>
#include <vector>

/**
 * A clickable region declared in scene_info.json.
 *
 * `target` is a data-root-relative path to the scene to navigate to when
 * this zone is clicked. GameRunner receives the zone and decides what state
 * transitions (if any) accompany the navigation.
 */
struct SceneZone
{
    std::string id;
    int         x      = 0;
    int         y      = 0;
    int         w      = 0;
    int         h      = 0;
    std::string target; // e.g. "/LEVELS/LEVEL_2/SCENE_1", or "" for no navigation
};

/**
 * A single scene within a level.
 * Loaded from LEVELS/<level_id>/SCENE_N/scene_info.json.
 *
 * Holds display and navigation data only — no game-state logic.
 * When a zone is clicked GameRunner receives the zone and handles all state transitions.
 */
struct LevelScene
{
    std::string           id;     // directory name, e.g. "SCENE_1"
    std::string           name;   // friendly display name
    std::string           md;     // data-root-relative path to .md file, or ""
    std::string           png;    // data-root-relative path to .png file, or ""
    std::vector<SceneZone> zones;
};
