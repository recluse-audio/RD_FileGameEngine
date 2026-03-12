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

        if (id == "scene_list")
        {
            mSceneListSection = std::make_unique<SceneListSection>(id, label, x, y, width, height);
            mSceneListSection->setColor(c[0], c[1], c[2]);
        }
        else if (id == "scene_view")
        {
            mActiveSceneSection = std::make_unique<ActiveSceneSection>(id, label, x, y, width, height);
            mActiveSceneSection->setColor(c[0], c[1], c[2]);
            mActiveSceneSection->setGameRunner(this);
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
    for (const std::string& levelDir : mFileOperator.listDirectory("/LEVELS"))
    {
        std::string infoJson = mFileOperator.load("/LEVELS/" + levelDir + "/level_info.json");
        nlohmann::json j = nlohmann::json::parse(infoJson, nullptr, false);
        if (j.is_discarded()) continue;

        Level level;
        level.id   = levelDir;
        level.name = j.value("name", levelDir);

        const std::string levelPath = "/LEVELS/" + levelDir;
        for (const std::string& sceneDir : mFileOperator.listDirectory(levelPath))
        {
            if (sceneDir.rfind("SCENE_", 0) != 0) continue;

            const std::string scenePath = levelPath + "/" + sceneDir;
            std::string sceneJson = mFileOperator.load(scenePath + "/scene_info.json");
            nlohmann::json sj = nlohmann::json::parse(sceneJson, nullptr, false);
            if (sj.is_discarded()) continue;

            LevelScene scene;
            scene.id       = sceneDir;
            scene.name     = sj.value("name",     sceneDir);
            scene.password = sj.value("password", "");

            std::string mdFile  = sj.value("md",  "");
            std::string pngFile = sj.value("png", "");
            if (!mdFile.empty())  scene.md  = scenePath + "/" + mdFile;
            if (!pngFile.empty()) scene.png = scenePath + "/" + pngFile;

            if (sj.contains("zones") && sj["zones"].is_array())
            {
                for (auto& z : sj["zones"])
                {
                    SceneZone zone;
                    zone.id     = z.value("id",     "");
                    zone.x      = z.value("x",      0);
                    zone.y      = z.value("y",      0);
                    zone.w      = z.value("w",      0);
                    zone.h      = z.value("h",      0);
                    zone.target = z.value("target", "");
                    if (z.contains("points") && z["points"].is_array())
                        for (auto& pt : z["points"])
                            if (pt.is_array() && pt.size() == 2)
                                zone.points.push_back({ pt[0].get<int>(), pt[1].get<int>() });
                    scene.zones.push_back(std::move(zone));
                }
            }

            level.scenes.push_back(std::move(scene));
        }

        mLevels.push_back(std::move(level));
    }
}

void GameRunner::loadInitialState()
{
    std::string stateJson = mFileOperator.load("/GAME_STATE/Default_Game_State.json");
    nlohmann::json state = nlohmann::json::parse(stateJson, nullptr, false);
    if (state.is_discarded()) { updateSceneList(); return; }
    applyStateJson(state);
}

void GameRunner::updateSceneList()
{
    if (!mSceneListSection || mLevels.empty()) return;

    const Level& level = mLevels[mActiveLevelIndex];

    std::vector<std::string> names;
    for (const auto& scene : level.scenes)
        names.push_back(scene.name);

    mSceneListSection->setScenes(names);
    updateActiveScene();
}

void GameRunner::updateActiveScene()
{
    if (!mActiveSceneSection || !mSceneListSection) return;

    int idx = mSceneListSection->getSelectedIndex();
    if (!mLevels.empty() && idx >= 0 && idx < (int)mLevels[mActiveLevelIndex].scenes.size())
        mActiveSceneSection->setActiveScene(&mLevels[mActiveLevelIndex].scenes[idx]);
    else
        mActiveSceneSection->setActiveScene(nullptr);
}

void GameRunner::drawTopBar()
{
    int dbgR = mDoDebugAction ? 60  : 40;
    int dbgG = mDoDebugAction ? 200 : 40;
    int dbgB = mDoDebugAction ? 80  : 40;
    mRenderer.drawFilledRect(k_DebugBtnX, 0, k_DebugBtnWidth, k_TopBarHeight, dbgR, dbgG, dbgB, 240);
    mRenderer.drawLabel("debug", k_DebugBtnX + 2, 6);

    int znR = mShowZones ? 60  : 40;
    int znG = mShowZones ? 120 : 40;
    int znB = mShowZones ? 200 : 40;
    mRenderer.drawFilledRect(k_ZonesBtnX, 0, k_ZonesBtnWidth, k_TopBarHeight, znR, znG, znB, 240);
    mRenderer.drawLabel("Z", k_ZonesBtnX + 6, 6);

    int ovR = mShowOverlay ? 200 : 40;
    int ovG = mShowOverlay ? 140 : 40;
    int ovB = mShowOverlay ? 60  : 40;
    mRenderer.drawFilledRect(k_OverlayBtnX, 0, k_OverlayBtnWidth, k_TopBarHeight, ovR, ovG, ovB, 240);
    mRenderer.drawLabel("Ovr", k_OverlayBtnX + 2, 6);

    mRenderer.drawButton("^", k_HomeBtnX, 0, k_HomeBtnWidth, k_TopBarHeight);

    if (mLevels.empty()) return;

    mRenderer.drawButton("<", 0, 0, k_NavBtnWidth, k_TopBarHeight);
    mRenderer.drawButton(">", 320 - k_NavBtnWidth, 0, k_NavBtnWidth, k_TopBarHeight);

    mRenderer.drawButton("Sv", k_SaveBtnX, 0, k_SaveBtnWidth, k_TopBarHeight);
    mRenderer.drawButton("Ld", k_LoadBtnX, 0, k_LoadBtnWidth, k_TopBarHeight);

    mRenderer.drawLabel(mLevels[mActiveLevelIndex].name, k_LoadBtnX + k_LoadBtnWidth + 4, 6);
}

void GameRunner::draw()
{
    for (const auto& section : mGUISections)
        section.draw(mRenderer, mDoDebugAction);

    if (mSceneListSection)
        mSceneListSection->draw(mRenderer, mDoDebugAction);

    if (mActiveSceneSection)
        mActiveSceneSection->draw(mRenderer, mDoDebugAction);

    drawTopBar();
}

void GameRunner::toggleDebug()
{
    mDoDebugAction = !mDoDebugAction;
}

void GameRunner::toggleZones()
{
    mShowZones = !mShowZones;
    if (mActiveSceneSection)
        mActiveSceneSection->setShowZones(mShowZones);
}

void GameRunner::toggleOverlay()
{
    mShowOverlay = !mShowOverlay;
    if (mActiveSceneSection)
        mActiveSceneSection->setShowOverlay(mShowOverlay);
}

void GameRunner::nextLevel()
{
    if (mLevels.empty()) return;
    mActiveLevelIndex = (mActiveLevelIndex + 1) % (int)mLevels.size();
    updateSceneList();
}

void GameRunner::prevLevel()
{
    if (mLevels.empty()) return;
    mActiveLevelIndex = (mActiveLevelIndex - 1 + (int)mLevels.size()) % (int)mLevels.size();
    updateSceneList();
}

void GameRunner::submitPassword(const std::string& entry)
{
    if (mLevels.empty() || !mSceneListSection) return;

    int idx = mSceneListSection->getSelectedIndex();
    auto& scenes = mLevels[mActiveLevelIndex].scenes;
    if (idx < 0 || idx >= (int)scenes.size()) return;

    LevelScene& scene = scenes[idx];
    if (entry == scene.password)
    {
        scene.isUnlocked = true;
        updateActiveScene();
    }
}

nlohmann::json GameRunner::buildStateJson() const
{
    nlohmann::json j;
    j["game_id"] = mGameId;

    if (!mLevels.empty())
    {
        j["current_level"] = mLevels[mActiveLevelIndex].name;
        j["current_scene"] = "";
        if (mSceneListSection)
        {
            int idx = mSceneListSection->getSelectedIndex();
            const auto& scenes = mLevels[mActiveLevelIndex].scenes;
            if (idx >= 0 && idx < (int)scenes.size())
                j["current_scene"] = scenes[idx].name;
        }
    }

    for (const auto& level : mLevels)
    {
        j["levels"][level.name]["isUnlocked"] = level.isUnlocked;
        for (const auto& scene : level.scenes)
            j["levels"][level.name]["scenes"][scene.name]["isUnlocked"] = scene.isUnlocked;
    }

    return j;
}

void GameRunner::applyStateJson(const nlohmann::json& state)
{
    if (state.contains("levels") && state["levels"].is_object())
    {
        for (auto& level : mLevels)
        {
            if (!state["levels"].contains(level.name)) continue;
            const auto& ls = state["levels"][level.name];
            level.isUnlocked = ls.value("isUnlocked", true);
            if (!ls.contains("scenes") || !ls["scenes"].is_object()) continue;
            for (auto& scene : level.scenes)
            {
                if (!ls["scenes"].contains(scene.name)) continue;
                scene.isUnlocked = ls["scenes"][scene.name].value("isUnlocked", true);
            }
        }
    }

    std::string currentLevel = state.value("current_level", "");
    std::string currentScene = state.value("current_scene", "");

    mActiveLevelIndex = 0;
    for (int i = 0; i < (int)mLevels.size(); ++i)
    {
        if (mLevels[i].name == currentLevel) { mActiveLevelIndex = i; break; }
    }

    updateSceneList();

    if (!mSceneListSection || currentScene.empty()) return;

    const auto& names = mSceneListSection->getScenes();
    for (int i = 0; i < (int)names.size(); ++i)
    {
        if (names[i] == currentScene) { mSceneListSection->setSelectedIndex(i); break; }
    }

    updateActiveScene();
}

std::string GameRunner::savePath() const
{
    return mSaveDir + "/" + mGameId + "_save.json";
}

void GameRunner::saveGame()
{
    if (mGameId.empty()) return;
    mFileOperator.writeAbsolute(savePath(), buildStateJson().dump(2));
}

bool GameRunner::loadGameFromPath(const std::string& path)
{
    std::string content = mFileOperator.loadAbsolute(path);
    if (content.empty()) return false;

    nlohmann::json state = nlohmann::json::parse(content, nullptr, false);
    if (state.is_discarded()) return false;
    if (state.value("game_id", "") != mGameId) return false;

    applyStateJson(state);
    return true;
}

void GameRunner::registerScroll(int delta)
{
    if (mActiveSceneSection)
        mActiveSceneSection->scroll(delta);
}

void GameRunner::registerHit(int x, int y)
{
    if (y >= 0 && y < k_TopBarHeight)
    {
        if (x >= 0 && x < k_NavBtnWidth)
            prevLevel();
        else if (x >= k_SaveBtnX && x < k_SaveBtnX + k_SaveBtnWidth)
            saveGame();
        else if (x >= k_LoadBtnX && x < k_LoadBtnX + k_LoadBtnWidth)
            mWantsToLoadGame = true;
        else if (x >= k_HomeBtnX && x < k_HomeBtnX + k_HomeBtnWidth)
            mWantsToExitToLibrary = true;
        else if (x >= k_ZonesBtnX && x < k_ZonesBtnX + k_ZonesBtnWidth)
            toggleZones();
        else if (x >= k_OverlayBtnX && x < k_OverlayBtnX + k_OverlayBtnWidth)
            toggleOverlay();
        else if (x >= k_DebugBtnX && x < k_DebugBtnX + k_DebugBtnWidth)
            toggleDebug();
        else if (x >= 320 - k_NavBtnWidth && x < 320)
            nextLevel();
    }
    else if (mSceneListSection && mSceneListSection->containsPoint(x, y))
    {
        mSceneListSection->registerHit(x, y);
        updateActiveScene();
    }
    else if (mActiveSceneSection && mActiveSceneSection->containsPoint(x, y))
    {
        mActiveSceneSection->registerHit(x, y);
    }
}
