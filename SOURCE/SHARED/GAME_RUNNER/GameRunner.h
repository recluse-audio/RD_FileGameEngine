/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "../GUI_SECTION/GUISection.h"
#include "../GUI_SECTION/SceneListSection.h"
#include "../GUI_SECTION/ActiveSceneSection.h"
#include "../LEVEL/Level.h"

class FileOperator;
class GraphicsRenderer;

/**
 * Top-level coordinator that owns the GUI layout sections and level list.
 * Loads section definitions from /GUI/gui_layout.json and level metadata
 * from each /LEVELS/<id>/level_info.json and its SCENE_N subdirectories
 * via FileOperator.
 * Renders sections and the top-bar level navigator each frame.
 */
class GameRunner
{
public:
    GameRunner(FileOperator& fileOperator, GraphicsRenderer& renderer);

    void draw();
    void registerHit(int x, int y);
    void registerScroll(int delta);
    void nextLevel();
    void prevLevel();
    void toggleDebug();
    void toggleZones();
    void toggleOverlay();
    void submitPassword(const std::string& entry);

    void setGameId(const std::string& id)  { mGameId   = id; }
    void setSaveDir(const std::string& dir) { mSaveDir  = dir; }

    /** Write current state to <saveDir>/<gameId>_save.json. */
    void saveGame();

    /**
     * Load a specific save file and apply it.
     * Returns false if the file is unreadable or game_id does not match.
     */
    bool loadGameFromPath(const std::string& path);

    bool        wantsToLoadGame()  const { return mWantsToLoadGame; }
    void        clearLoadGameFlag()      { mWantsToLoadGame = false; }
    std::string getGameId()        const { return mGameId; }
    std::string getSaveDir()       const { return mSaveDir; }

    const std::vector<GUISection>& getSections()            const { return mGUISections; }
    const std::vector<Level>&      getLevels()              const { return mLevels; }
    SceneListSection*              getSceneListSection()          { return mSceneListSection.get(); }
    const SceneListSection*        getSceneListSection()    const { return mSceneListSection.get(); }
    ActiveSceneSection*            getActiveSceneSection()        { return mActiveSceneSection.get(); }
    const ActiveSceneSection*      getActiveSceneSection()  const { return mActiveSceneSection.get(); }
    int                            getActiveLevelIndex()    const { return mActiveLevelIndex; }
    bool                           getDoDebugAction()       const { return mDoDebugAction; }
    bool                           wantsToExitToLibrary()   const { return mWantsToExitToLibrary; }

private:
    void loadSections();
    void loadLevels();
    void loadInitialState();
    void drawTopBar();
    void updateSceneList();
    void updateActiveScene();

    /** Serialize current state to a JSON object (for saving). */
    nlohmann::json buildStateJson() const;

    /** Apply a parsed state JSON (level/scene unlock flags, current position). */
    void applyStateJson(const nlohmann::json& state);

    std::string savePath() const;

    static constexpr int k_TopBarHeight  = 20;
    static constexpr int k_NavBtnWidth   = 20;
    static constexpr int k_SaveBtnWidth  = 18;
    static constexpr int k_SaveBtnX      = k_NavBtnWidth;
    static constexpr int k_LoadBtnWidth  = 18;
    static constexpr int k_LoadBtnX      = k_SaveBtnX + k_SaveBtnWidth;
    static constexpr int k_DebugBtnWidth = 36;
    static constexpr int k_DebugBtnX     = 320 - k_NavBtnWidth - k_DebugBtnWidth;
    static constexpr int k_ZonesBtnWidth = 20;
    static constexpr int k_ZonesBtnX     = k_DebugBtnX - k_ZonesBtnWidth;
    static constexpr int k_HomeBtnWidth    = 20;
    static constexpr int k_HomeBtnX       = k_ZonesBtnX - k_HomeBtnWidth;
    static constexpr int k_OverlayBtnWidth = 28;
    static constexpr int k_OverlayBtnX    = k_HomeBtnX - k_OverlayBtnWidth;

    FileOperator&                        mFileOperator;
    GraphicsRenderer&                    mRenderer;
    std::vector<GUISection>              mGUISections;
    std::unique_ptr<SceneListSection>    mSceneListSection;
    std::unique_ptr<ActiveSceneSection>  mActiveSceneSection;
    std::vector<Level>                   mLevels;
    int                                  mActiveLevelIndex      = 0;
    bool                                 mDoDebugAction         = false;
    bool                                 mShowZones             = false;
    bool                                 mShowOverlay           = false;
    bool                                 mWantsToExitToLibrary  = false;
    std::string                          mGameId;
    std::string                          mSaveDir               = "C:/FILE_GAME_SAVES";
    bool                                 mWantsToLoadGame       = false;
};
