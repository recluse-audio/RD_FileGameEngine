#include "GameRunner.h"
#include "../FILE_OPERATOR/FileOperator.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "../SCENE/Scene.h"
#include <algorithm>
#include <nlohmann/json.hpp>

GameRunner::GameRunner(FileOperator& fileParser, GraphicsRenderer& renderer,
                       std::string mode, std::string locationID,
                       std::string saveDir, bool useHires)
: mFileOperator(fileParser)
, mRenderer(renderer)
, mSceneView(renderer)
, mTopBar(renderer)
, mBottomBar(renderer)
, mSceneFactory(useHires)
, mGameStartManager(fileParser, std::move(saveDir))
, mCurrentMode(mode)
, mCurrentLocationID(locationID)
{
    mTopBar.load(mFileOperator.load("/GUI/Top_Bar.json"));
    mBottomBar.load(mFileOperator.load("/GUI/Bottom_Bar.json"));
}

void GameRunner::draw()
{
    if (!mActiveScene) return;
    mRenderer.setScrollOffset(mScrollOffset);
    mSceneView.draw(*mActiveScene, mOverlayVisible, mZoneDisplayVisible);
    if (mFileMenuVisible && mFileMenuScene)
        mSceneView.drawMenu(*mFileMenuScene);
    // Bars drawn last so they are never obstructed by scene content.
    mTopBar.draw();
    mBottomBar.draw();
}

void GameRunner::registerHit(int x, int y)
{
    if (!mActiveScene) return;

    std::string cb = mTopBar.handleHit(x, y);
    if (cb.empty()) cb = mBottomBar.handleHit(x, y);
    if (!cb.empty())
    {
        dispatchCallback(cb);
        return;
    }

    if (mFileMenuVisible && mFileMenuScene)
    {
        std::string target = mFileMenuScene->getInterceptingZoneTarget(x, y);
        if (!target.empty())
        {
            mFileMenuVisible = false;
            loadScene(target);
            return;
        }
        mFileMenuVisible = false;
        return;
    }

    std::string noteTarget = mActiveScene->getInterceptingZoneNoteTarget(x, y);
    if (!noteTarget.empty())
    {
        discoverNote(noteTarget);
        return;
    }

    std::string target = mActiveScene->getInterceptingZoneTarget(x, y);
    if (!target.empty())
    {
        loadScene(target);
        return;
    }

    std::string zoneId = mActiveScene->getInterceptingZoneID(x, y);
    if (!zoneId.empty())
        dispatchCallback(zoneId);
}

void GameRunner::dispatchCallback(const std::string& callbackId)
{
    if (callbackId == "toggleOverlay")
    {
        mOverlayVisible = !mOverlayVisible;
        syncControlsState();
    }
    else if (callbackId == "toggleZoneDisplay")
    {
        mZoneDisplayVisible = !mZoneDisplayVisible;
    }
    else if (callbackId == "navigateUp")
    {
        std::string parent = mActiveScene->getParentPath();
        if (!parent.empty()) loadScene(parent);
    }
    else if (callbackId == "navigatePrev")
    {
        if (mCurrentMode == "notes" && !mNoteList.empty())
        {
            mNoteIndex = (mNoteIndex - 1 + (int)mNoteList.size()) % (int)mNoteList.size();
            loadNote(mNoteList[mNoteIndex]);
        }
    }
    else if (callbackId == "navigateNext")
    {
        if (mCurrentMode == "notes" && !mNoteList.empty())
        {
            mNoteIndex = (mNoteIndex + 1) % (int)mNoteList.size();
            loadNote(mNoteList[mNoteIndex]);
        }
    }
    else if (callbackId == "switchToLocations")
    {
        mCurrentMode = "locations";
        if (!mLastLocationPath.empty())
            loadScene(mLastLocationPath);
        else
            syncControlsState();
    }
    else if (callbackId == "switchToNotes")
    {
        mCurrentMode = "notes";
        if (mNoteList.empty())
        {
            std::string stateJson = mFileOperator.load("/GAME_STATE/Game_State.json");
            nlohmann::json j = nlohmann::json::parse(stateJson, nullptr, false);
            if (!j.is_discarded() && j.contains("notes"))
                for (auto& n : j["notes"])
                    mNoteList.push_back(n.get<std::string>());
        }
        if (!mNoteList.empty())
            loadNote(mNoteList[mNoteIndex]);
        else
            syncControlsState();
    }
    else if (callbackId == "open_file_manager")
    {
        if (!mFileMenuScene)
        {
            std::string json = mFileOperator.load("/LOCATIONS/AVERY/DESK/COMPUTER/FILE_MENU/File_Menu.json");
            mFileMenuScene = mSceneFactory.build(json);
        }
        mFileMenuVisible = !mFileMenuVisible;
    }
    else if (callbackId == "start_button")
    {
        mGameStartManager.save();
        loadScene("/LOCATIONS/AVERY/ROOT/Avery_Full.json");
    }
}

void GameRunner::syncControlsState()
{
    BarState state;
    state.isRoot         = mActiveScene ? mActiveScene->isRoot() : false;
    state.overlayVisible = mOverlayVisible;
    state.hasParent      = mActiveScene && !mActiveScene->getParentPath().empty();
    state.mode           = mCurrentMode;
    mTopBar.setState(state);
    mBottomBar.setState(state);
}

void GameRunner::discoverSceneNote(const std::string& scenePath, const std::string& sceneJson)
{
    std::string clueText = mFileOperator.load(mActiveScene->getSecondaryPath());
    if (!clueText.empty())
        mFileOperator.appendToFile(mActiveScene->getNoteTarget(), clueText);

    nlohmann::json j = nlohmann::json::parse(sceneJson, nullptr, false);
    if (!j.is_discarded())
    {
        j["isDiscovered"] = true;
        mFileOperator.writeToFile(scenePath, j.dump(2));
    }
    mActiveScene->setIsDiscovered(true);
}

void GameRunner::loadScene(const std::string& path)
{
    mOverlayVisible  = false;
    mFileMenuVisible = false;
    mScrollOffset    = 0;
    std::string json = mFileOperator.load(path);

#ifdef ARDUINO
    Serial.printf("[GR] loadScene: %s  json=%d bytes\n",
        path.c_str(), (int)json.size());
#endif

    mActiveScene = mSceneFactory.build(json);

    if (mCurrentMode == "locations")
        mLastLocationPath = path;

    if (!mActiveScene->isDiscovered() && !mActiveScene->getNoteTarget().empty())
        discoverSceneNote(path, json);

    syncControlsState();
}

void GameRunner::loadNote(const std::string& mdPath)
{
    mOverlayVisible  = false;
    mFileMenuVisible = false;
    mScrollOffset    = 0;
    mActiveScene     = std::make_unique<Scene>("NOTE", "", "", mdPath, "");
    syncControlsState();
}

void GameRunner::discoverNote(const std::string& notePath)
{
    std::string json = mFileOperator.load(notePath);
    nlohmann::json j = nlohmann::json::parse(json, nullptr, false);
    if (j.is_discarded()) return;
    j["isDiscovered"] = true;
    mFileOperator.writeToFile(notePath, j.dump(2));
}

void GameRunner::refreshNote(const std::string& clueArrayKey)
{
    std::string stateJson = mFileOperator.load("/GAME_STATE/Game_State.json");
    nlohmann::json state = nlohmann::json::parse(stateJson, nullptr, false);
    if (state.is_discarded()) return;

    auto& configs = state["note_configs"];
    if (!configs.contains(clueArrayKey)) return;

    std::string notePath = configs[clueArrayKey].value("note_path", "");
    std::string basePath = configs[clueArrayKey].value("base_path", "");
    if (notePath.empty()) return;

    mFileOperator.writeToFile(notePath, mFileOperator.load(basePath));

    auto& clues = state[clueArrayKey];
    for (auto it = clues.begin(); it != clues.end(); ++it)
    {
        if (!it.value().get<bool>()) continue;

        std::string clueJson = mFileOperator.load(it.key());
        nlohmann::json clue = nlohmann::json::parse(clueJson, nullptr, false);
        if (clue.is_discarded()) continue;

        std::string secondaryPath = clue.value("secondary_path", "");
        if (!secondaryPath.empty())
            mFileOperator.appendToFile(notePath, mFileOperator.load(secondaryPath));
    }
}

void GameRunner::setSaveDir(const std::string& dir)
{
    mGameStartManager.setSaveDir(dir);
}

void GameRunner::scroll(int delta)
{
    mScrollOffset = std::max(0, mScrollOffset + delta);
}

std::string GameRunner::getCurrentMode() const
{
    return mCurrentMode;
}

std::string GameRunner::getCurrentLocationID() const
{
    return mCurrentLocationID;
}

std::string GameRunner::getCurrentNoteID() const
{
    return mNoteIndex < (int)mNoteList.size() ? mNoteList[mNoteIndex] : "";
}

bool GameRunner::isFileMenuVisible() const
{
    return mFileMenuVisible;
}
