#include <catch2/catch_test_macros.hpp>
#include "GAME_RUNNER/GameRunner.h"
#include "GUI_SECTION/GUISection.h"
#include "LEVEL/Level.h"
#include "UTIL/TestFileOperator.h"
#include "UTIL/NullGraphicsRenderer.h"

TEST_CASE("GameRunner constructs without error", "[GameRunner]")
{
    TestFileOperator     fileOp;
    NullGraphicsRenderer renderer;

    // gui_layout.json is not present — construction must not crash.
    REQUIRE_NOTHROW(GameRunner(fileOp, renderer));
}

TEST_CASE("GameRunner loads sections from gui_layout.json", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "top_bar":    { "label": "Top Bar",    "x": 0,  "y": 0,  "width": 320, "height": 20  },
            "scene_list": { "label": "Scene List", "x": 0,  "y": 20, "width": 64,  "height": 220 },
            "scene_view": { "label": "Scene View", "x": 64, "y": 20, "width": 256, "height": 220 }
        }
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    // scene_list and scene_view are held separately; only top_bar remains as a plain section.
    REQUIRE(runner.getSections().size() == 1);

    SECTION("plain section IDs are present")
    {
        const auto& sections = runner.getSections();
        auto hasId = [&](const std::string& id)
        {
            for (const auto& s : sections)
                if (s.getId() == id) return true;
            return false;
        };
        CHECK(hasId("top_bar"));
    }

    SECTION("scene_list section is created separately")
    {
        REQUIRE(runner.getSceneListSection() != nullptr);
        CHECK(runner.getSceneListSection()->getId() == "scene_list");
    }

    SECTION("section bounds are correct")
    {
        const auto& sections = runner.getSections();
        auto find = [&](const std::string& id) -> const GUISection* {
            for (const auto& s : sections)
                if (s.getId() == id) return &s;
            return nullptr;
        };
        const GUISection* topBar = find("top_bar");
        REQUIRE(topBar != nullptr);
        CHECK(topBar->getX()      == 0);
        CHECK(topBar->getY()      == 0);
        CHECK(topBar->getWidth()  == 320);
        CHECK(topBar->getHeight() == 20);
    }
}

TEST_CASE("GameRunner loads levels and scenes from LEVELS directory", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name": "Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({
        "name": "Scene 1",
        "md": "content.md"
    })";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_2/scene_info.json"] = R"({
        "name": "Scene 2",
        "png": "image.png"
    })";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name": "Level 2"})";
    fileOp.files["/LEVELS/LEVEL_2/SCENE_1/scene_info.json"] = R"({
        "name": "The Scene",
        "md": "notes.md",
        "png": "scene.png"
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getLevels().size() == 2);

    SECTION("level IDs and names are correct")
    {
        CHECK(runner.getLevels()[0].id   == "LEVEL_1");
        CHECK(runner.getLevels()[0].name == "Level 1");
        CHECK(runner.getLevels()[1].id   == "LEVEL_2");
        CHECK(runner.getLevels()[1].name == "Level 2");
    }

    SECTION("scenes are loaded per level")
    {
        const auto& s1 = runner.getLevels()[0].scenes;
        REQUIRE(s1.size() == 2);
        CHECK(s1[0].id   == "SCENE_1");
        CHECK(s1[0].name == "Scene 1");
        CHECK(s1[0].md   == "/LEVELS/LEVEL_1/SCENE_1/content.md");
        CHECK(s1[0].png  == "");
        CHECK(s1[1].id   == "SCENE_2");
        CHECK(s1[1].name == "Scene 2");
        CHECK(s1[1].png  == "/LEVELS/LEVEL_1/SCENE_2/image.png");
        CHECK(s1[1].md   == "");

        const auto& s2 = runner.getLevels()[1].scenes;
        REQUIRE(s2.size() == 1);
        CHECK(s2[0].name == "The Scene");
        CHECK(s2[0].md   == "/LEVELS/LEVEL_2/SCENE_1/notes.md");
        CHECK(s2[0].png  == "/LEVELS/LEVEL_2/SCENE_1/scene.png");
    }
}

TEST_CASE("GameRunner loads scene zones with targets", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name": "Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({
        "name": "Scene 1",
        "png": "scene.png",
        "zones": [
            { "id": "door", "x": 10, "y": 20, "w": 40, "h": 30, "target": "/LEVELS/LEVEL_2/SCENE_1" },
            { "id": "box",  "x": 80, "y": 50, "w": 20, "h": 20, "target": "" }
        ]
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getLevels().size() == 1);
    const auto& zones = runner.getLevels()[0].scenes[0].zones;
    REQUIRE(zones.size() == 2);

    CHECK(zones[0].id     == "door");
    CHECK(zones[0].x      == 10);
    CHECK(zones[0].y      == 20);
    CHECK(zones[0].w      == 40);
    CHECK(zones[0].h      == 30);
    CHECK(zones[0].target == "/LEVELS/LEVEL_2/SCENE_1");

    CHECK(zones[1].id     == "box");
    CHECK(zones[1].target == "");
}

TEST_CASE("SceneListSection shows scenes for active level", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "scene_list": { "label": "Scene List", "x": 0, "y": 20, "width": 64, "height": 220 }
        }
    })";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"First Scene"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_2/scene_info.json"] = R"({"name":"Second Scene"})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2"})";
    fileOp.files["/LEVELS/LEVEL_2/SCENE_1/scene_info.json"] = R"({"name":"Only Scene"})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    SceneListSection* list = runner.getSceneListSection();
    REQUIRE(list != nullptr);

    SECTION("initial level scenes are loaded")
    {
        REQUIRE(list->getScenes().size() == 2);
        CHECK(list->getScenes()[0] == "First Scene");
        CHECK(list->getScenes()[1] == "Second Scene");
    }

    SECTION("scenes update when level changes")
    {
        runner.nextLevel();
        REQUIRE(list->getScenes().size() == 1);
        CHECK(list->getScenes()[0] == "Only Scene");
    }

    SECTION("selection resets to first scene when level changes")
    {
        list->setSelectedIndex(1);
        runner.nextLevel();
        CHECK(list->getSelectedIndex() == 0);
    }

    SECTION("setSelectedIndex updates selected name")
    {
        list->setSelectedIndex(0);
        CHECK_FALSE(list->getSelectedName().empty());
    }
}

TEST_CASE("GameRunner restores active level and scene from Default_Game_State.json", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"scene_list":{"label":"Scene List","x":0,"y":20,"width":64,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"First Scene"})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2"})";
    fileOp.files["/LEVELS/LEVEL_2/SCENE_1/scene_info.json"] = R"({"name":"Only Scene"})";
    fileOp.files["/GAME_STATE/Default_Game_State.json"] = R"({"current_level":"Level 2","current_scene":"Only Scene","levels":{}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    CHECK(runner.getActiveLevelIndex() == 1);
    REQUIRE(runner.getSceneListSection() != nullptr);
    CHECK(runner.getSceneListSection()->getSelectedName() == "Only Scene");
}

TEST_CASE("GameRunner top-bar navigation changes active level", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2"})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveLevelIndex() == 0);

    SECTION("next advances to following level")
    {
        runner.nextLevel();
        CHECK(runner.getActiveLevelIndex() == 1);
    }

    SECTION("prev wraps to last level from first")
    {
        runner.prevLevel();
        CHECK(runner.getActiveLevelIndex() == 1);
    }

    SECTION("next wraps back to first level")
    {
        runner.nextLevel();
        runner.nextLevel();
        CHECK(runner.getActiveLevelIndex() == 0);
    }
}

TEST_CASE("GameRunner debug toggle flips mDoDebugAction", "[GameRunner]")
{
    TestFileOperator     fileOp;
    NullGraphicsRenderer renderer;
    GameRunner           runner(fileOp, renderer);

    REQUIRE(runner.getDoDebugAction() == false);

    SECTION("toggleDebug turns it on")
    {
        runner.toggleDebug();
        CHECK(runner.getDoDebugAction() == true);
    }

    SECTION("toggleDebug twice returns to false")
    {
        runner.toggleDebug();
        runner.toggleDebug();
        CHECK(runner.getDoDebugAction() == false);
    }
}
