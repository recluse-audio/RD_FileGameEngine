/**
 * Made by Ryan Devens on 2026-03-07
 */

#pragma once
#include "GUISection.h"
#include "../LEVEL/LevelScene.h"
#include <string>
#include <vector>

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
    void setShowZones(bool show) { mShowZones = show; }

    void draw(GraphicsRenderer& renderer, bool showLabel = false) const override;

    std::string getPngPath() const { return mPngPath; }
    std::string getMdPath()  const { return mMdPath; }

private:
    std::string              mPngPath;
    std::string              mMdPath;
    std::vector<SceneZone>   mZones;
    bool                     mShowZones = false;
};
