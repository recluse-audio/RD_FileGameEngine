/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include <string>
#include <memory>
#include "Scene.h"

/**
 * Parses a scene JSON string and constructs a fully populated Scene.
 * Used by GameRunner to build the active scene from file content.
 *
 * Zone coordinates in JSON must be in 320x240 game space.
 */
class SceneFactory
{
public:
    /**
     * useHires — if true, primaryPath is read from "hires_image_path";
     *            if false (default), from "lores_image_path".
     */
    explicit SceneFactory(bool useHires = false);

    std::unique_ptr<Scene> build(const std::string& jsonString);

private:
    bool mUseHires;
};
