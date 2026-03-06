#include "GameRunner.h"
#include "../FILE_OPERATOR/FileOperator.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include <nlohmann/json.hpp>

GameRunner::GameRunner(FileOperator& fileOperator, GraphicsRenderer& renderer)
: mFileOperator(fileOperator)
, mRenderer(renderer)
{
    loadSections();
    loadLevels();
    loadInitialState();
}

void GameRunner::loadSections()
{
    static const int palette[][3] =
    {
        { 220,  80,  80 },  // red
        {  80, 150, 220 },  // blue
        {  80, 200, 120 },  // green
        { 200, 160,  60 },  // amber
        { 160,  80, 200 },  // purple
    };

    std::string layoutJson = mFileOperator.load("/GUI/gui_layout.json");
    nlohmann::json layout = nlohmann::json::parse(layoutJson, nullptr, false);
    if (layout.is_discarded() || !layout.contains("sections"))
        return;

    int colorIndex = 0;
    for (auto& [id, s] : layout["sections"].items())
    {
        std::string label  = s.value("label",  "");
        int         x      = s.value("x",      0);
        int         y      = s.value("y",      0);
        int         width  = s.value("width",  0);
        int         height = s.value("height", 0);
        const int*  c      = palette[colorIndex % 5];

        if (id == "asset_list")
        {
            mAssetListSection = std::make_unique<AssetListSection>(id, label, x, y, width, height);
            mAssetListSection->setColor(c[0], c[1], c[2]);
        }
        else if (id == "asset_view")
        {
            mActiveAssetSection = std::make_unique<ActiveAssetSection>(id, label, x, y, width, height);
            mActiveAssetSection->setColor(c[0], c[1], c[2]);
        }
        else
        {
            GUISection section(id, label, x, y, width, height);
            section.setColor(c[0], c[1], c[2]);
            mGUISections.push_back(std::move(section));
        }
        ++colorIndex;
    }
}

void GameRunner::loadLevels()
{
    for (const std::string& dirName : mFileOperator.listDirectory("/LEVELS"))
    {
        std::string infoJson = mFileOperator.load("/LEVELS/" + dirName + "/level_info.json");
        nlohmann::json j = nlohmann::json::parse(infoJson, nullptr, false);
        if (j.is_discarded()) continue;

        Level level;
        level.id   = dirName;
        level.name = j.value("name", dirName);

        if (j.contains("assets") && j["assets"].is_object())
            for (auto& [path, friendly] : j["assets"].items())
                level.assets[path] = friendly.get<std::string>();

        mLevels.push_back(std::move(level));
    }
}

void GameRunner::loadInitialState()
{
    std::string stateJson = mFileOperator.load("/GAME_STATE/Default_Game_State.json");
    nlohmann::json state = nlohmann::json::parse(stateJson, nullptr, false);
    if (state.is_discarded()) { updateAssetList(); return; }

    std::string currentLevel = state.value("current_level", "");
    std::string currentAsset = state.value("current_asset", "");

    for (int i = 0; i < (int)mLevels.size(); ++i)
    {
        if (mLevels[i].name == currentLevel)
        {
            mActiveLevelIndex = i;
            break;
        }
    }

    updateAssetList();

    if (!mAssetListSection || currentAsset.empty()) return;

    const auto& names = mAssetListSection->getAssets();
    for (int i = 0; i < (int)names.size(); ++i)
    {
        if (names[i] == currentAsset)
        {
            mAssetListSection->setSelectedIndex(i);
            break;
        }
    }

    updateActiveAsset();
}

void GameRunner::updateAssetList()
{
    if (!mAssetListSection || mLevels.empty()) return;

    const Level& level = mLevels[mActiveLevelIndex];

    std::vector<std::string> names;
    for (const auto& [path, friendly] : level.assets)
        names.push_back(friendly);

    mAssetListSection->setAssets(names);

    if (mActiveAssetSection)
    {
        mActiveAssetSection->setLevelAssets(level.assets);
        updateActiveAsset();
    }
}

void GameRunner::updateActiveAsset()
{
    if (!mActiveAssetSection || !mAssetListSection) return;
    mActiveAssetSection->setActiveAsset(mAssetListSection->getSelectedName());
}

void GameRunner::drawTopBar()
{
    int dbgR = mDoDebugAction ? 60  : 40;
    int dbgG = mDoDebugAction ? 200 : 40;
    int dbgB = mDoDebugAction ? 80  : 40;
    mRenderer.drawFilledRect(k_DebugBtnX, 0, k_DebugBtnWidth, k_TopBarHeight, dbgR, dbgG, dbgB, 240);
    mRenderer.drawLabel("debug", k_DebugBtnX + 2, 6);

    if (mLevels.empty()) return;

    mRenderer.drawButton("<", 0, 0, k_NavBtnWidth, k_TopBarHeight);
    mRenderer.drawButton(">", 320 - k_NavBtnWidth, 0, k_NavBtnWidth, k_TopBarHeight);
    mRenderer.drawLabel(mLevels[mActiveLevelIndex].name, k_NavBtnWidth + 4, 6);
}

void GameRunner::draw()
{
    for (const auto& section : mGUISections)
        section.draw(mRenderer, mDoDebugAction);

    if (mAssetListSection)
        mAssetListSection->draw(mRenderer, mDoDebugAction);

    if (mActiveAssetSection)
        mActiveAssetSection->draw(mRenderer, mDoDebugAction);

    drawTopBar();
}

void GameRunner::toggleDebug()
{
    mDoDebugAction = !mDoDebugAction;
}

void GameRunner::nextLevel()
{
    if (mLevels.empty()) return;
    mActiveLevelIndex = (mActiveLevelIndex + 1) % (int)mLevels.size();
    updateAssetList();
}

void GameRunner::prevLevel()
{
    if (mLevels.empty()) return;
    mActiveLevelIndex = (mActiveLevelIndex - 1 + (int)mLevels.size()) % (int)mLevels.size();
    updateAssetList();
}

void GameRunner::registerHit(int x, int y)
{
    if (y >= 0 && y < k_TopBarHeight)
    {
        if (x >= 0 && x < k_NavBtnWidth)
            prevLevel();
        else if (x >= k_DebugBtnX && x < k_DebugBtnX + k_DebugBtnWidth)
            toggleDebug();
        else if (x >= 320 - k_NavBtnWidth && x < 320)
            nextLevel();
    }
    else if (mAssetListSection && mAssetListSection->containsPoint(x, y))
    {
        mAssetListSection->registerHit(x, y);
        updateActiveAsset();
    }
}
