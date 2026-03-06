/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include <memory>
#include <vector>
#include "../GUI_SECTION/GUISection.h"
#include "../GUI_SECTION/AssetListSection.h"
#include "../GUI_SECTION/ActiveAssetSection.h"
#include "../LEVEL/Level.h"

class FileOperator;
class GraphicsRenderer;

/**
 * Top-level coordinator that owns the GUI layout sections and level list.
 * Loads section definitions from /GUI/gui_layout.json and level metadata
 * from each /LEVELS/<id>/level_info.json via FileOperator.
 * Renders sections and the top-bar level navigator each frame.
 */
class GameRunner
{
public:
    GameRunner(FileOperator& fileOperator, GraphicsRenderer& renderer);

    void draw();
    void registerHit(int x, int y);
    void nextLevel();
    void prevLevel();
    void toggleDebug();

    const std::vector<GUISection>&  getSections()            const { return mGUISections; }
    const std::vector<Level>&       getLevels()              const { return mLevels; }
    AssetListSection*               getAssetListSection()          { return mAssetListSection.get(); }
    const AssetListSection*         getAssetListSection()    const { return mAssetListSection.get(); }
    ActiveAssetSection*             getActiveAssetSection()        { return mActiveAssetSection.get(); }
    const ActiveAssetSection*       getActiveAssetSection()  const { return mActiveAssetSection.get(); }
    int                             getActiveLevelIndex()    const { return mActiveLevelIndex; }
    bool                            getDoDebugAction()       const { return mDoDebugAction; }

private:
    void loadSections();
    void loadLevels();
    void loadInitialState();
    void drawTopBar();
    void updateAssetList();
    void updateActiveAsset();

    static constexpr int k_TopBarHeight  = 20;
    static constexpr int k_NavBtnWidth   = 20;
    static constexpr int k_DebugBtnWidth = 36;
    static constexpr int k_DebugBtnX     = 320 - k_NavBtnWidth - k_DebugBtnWidth;

    FileOperator&                      mFileOperator;
    GraphicsRenderer&                  mRenderer;
    std::vector<GUISection>            mGUISections;
    std::unique_ptr<AssetListSection>  mAssetListSection;
    std::unique_ptr<ActiveAssetSection> mActiveAssetSection;
    std::vector<Level>                 mLevels;
    int                                mActiveLevelIndex = 0;
    bool                               mDoDebugAction    = false;
};
