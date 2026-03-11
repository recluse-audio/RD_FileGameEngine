/**
 * Made by Ryan Devens on 2026-03-07
 */

#pragma once
#include "GUISection.h"
#include "PasswordEntryComponent.h"
#include "../LEVEL/LevelScene.h"
#include <algorithm>
#include <string>
#include <vector>

class GameRunner;

/**
 * A GUISection that renders the currently selected scene's content.
 *
 * Call setActiveScene() with a pointer to the LevelScene to display.
 * If the scene has a .png it is drawn via drawImage.
 * If it has only a .md it is drawn via drawText.
 * Pass nullptr to clear the display.
 */
class ActiveSceneSection : public GUISection
{
public:
    using GUISection::GUISection;

    void setActiveScene(const LevelScene* scene);
    void setShowZones(bool show)    { mShowZones    = show; }
    void setShowOverlay(bool show)  { mShowOverlay  = show; }
    void setGameRunner(GameRunner* gr) { mGameRunner = gr; }
    void scroll(int delta) { if (!mMdPath.empty()) mMdScrollOffset = std::max(0, mMdScrollOffset + delta); }

    void draw(GraphicsRenderer& renderer, bool showLabel = false) const override;
    void registerHit(int x, int y);

    std::string getPngPath() const { return mPngPath; }
    std::string getMdPath()  const { return mMdPath; }

private:
    std::string              mPngPath;
    std::string              mMdPath;
    std::vector<SceneZone>   mZones;
    bool                          mShowZones   = false;
    bool                          mShowOverlay = false;
    bool                          mIsLocked    = false;
    std::string              mPassword;
    PasswordEntryComponent   mPasswordEntry;
    GameRunner*              mGameRunner    = nullptr;
    int                      mSubmitBtnX    = 0;
    int                      mSubmitBtnY    = 0;
    int                      mSubmitBtnW    = 0;
    int                      mSubmitBtnH    = 0;
    // TESTING ONLY - remove before final product
    int                      mSkipBtnX      = 0;
    int                      mSkipBtnY      = 0;
    int                      mSkipBtnW      = 0;
    int                      mSkipBtnH      = 0;
    int                      mMdScrollOffset = 0;
};
