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
            scene.id   = sceneDir;
            scene.name = sj.value("name", sceneDir);

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

    std::string currentLevel = state.value("current_level", "");
    std::string currentScene = state.value("current_scene", "");

    for (int i = 0; i < (int)mLevels.size(); ++i)
    {
        if (mLevels[i].name == currentLevel)
        {
            mActiveLevelIndex = i;
            break;
        }
    }

    updateSceneList();

    if (!mSceneListSection || currentScene.empty()) return;

    const auto& names = mSceneListSection->getScenes();
    for (int i = 0; i < (int)names.size(); ++i)
    {
        if (names[i] == currentScene)
        {
            mSceneListSection->setSelectedIndex(i);
            break;
        }
    }

    updateActiveScene();
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

    if (mLevels.empty()) return;

    mRenderer.drawButton("<", 0, 0, k_NavBtnWidth, k_TopBarHeight);
    mRenderer.drawButton(">", 320 - k_NavBtnWidth, 0, k_NavBtnWidth, k_TopBarHeight);
    mRenderer.drawLabel(mLevels[mActiveLevelIndex].name, k_NavBtnWidth + 4, 6);
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
    else if (mSceneListSection && mSceneListSection->containsPoint(x, y))
    {
        mSceneListSection->registerHit(x, y);
        updateActiveScene();
    }
}
