/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../SCENE/SceneFactory.h"
#include "../SCENE_VIEW/SceneView.h"
#include "../BAR/ControlBarSection.h"
#include "GameStartManager.h"

class FileOperator;
class GraphicsRenderer;
class Scene;

/**
 * Owns the active game state and the active Scene. Uses an injected FileOperator
 * to load scene JSON from storage and a SceneFactory to build the Scene.
 * Delegates scene rendering to SceneView and controls rendering to ControlsView,
 * both of which use an injected GraphicsRenderer.
 */
class GameRunner
{
public:
    GameRunner(FileOperator&       fileParser,
               GraphicsRenderer& renderer,
               std::string       mode       = "locations",
               std::string       locationID = "",
               std::string       saveDir    = "",
               bool              useHires   = false);

    /**
     * Render the active scene and controls via their respective views.
     */
    void draw();

    /**
     * Called by platform input callbacks when the user taps/clicks at (x, y).
     * Checks control buttons first, then zone hit-testing on the active scene.
     */
    void registerHit(int x, int y);

    /**
     * Load a scene from the given data-root-relative JSON path, build it via
     * SceneFactory, and take ownership as the new active scene.
     */
    void loadScene(const std::string& path);

    void setSaveDir(const std::string& dir);
    void scroll(int delta);

    std::string getCurrentMode()       const;
    std::string getCurrentLocationID() const;
    std::string getCurrentNoteID()     const;
    bool        isFileMenuVisible()    const;

private:
    FileOperator&          mFileOperator;
    GraphicsRenderer&      mRenderer;
    SceneView              mSceneView;
    ControlBarSection      mTopBar;
    ControlBarSection      mBottomBar;
    SceneFactory           mSceneFactory;
    GameStartManager       mGameStartManager;
    std::unique_ptr<Scene> mActiveScene;
    std::unique_ptr<Scene> mFileMenuScene;
    bool                   mOverlayVisible      = false;
    bool                   mFileMenuVisible     = false;
    bool                   mZoneDisplayVisible  = false;
    int                    mScrollOffset    = 0;

    std::string              mCurrentMode;
    std::string              mCurrentLocationID;
    std::string              mLastLocationPath;
    std::vector<std::string> mNoteList;
    int                      mNoteIndex = 0;

    void loadNote(const std::string& mdPath);
    void discoverNote(const std::string& notePath);
    void discoverSceneNote(const std::string& scenePath, const std::string& sceneJson);
    void refreshNote(const std::string& clueArrayKey);
    void dispatchCallback(const std::string& callbackId);
    void syncControlsState();
};
