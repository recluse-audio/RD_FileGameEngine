/**
 * Made by Ryan Devens on 2026-02-24
 */

#pragma once
#include "../NOTES/Notes.h"
#include "../SCENE/Scene.h"
/**
 *
 */
class GameState
{
public:

    const Notes& getNotes() { return mNotes; }
    const Scene& getScene() { return mCurrentScene; }
private:
    Notes mNotes;
    Scene mCurrentScene;
};
