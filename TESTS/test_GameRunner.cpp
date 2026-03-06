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

    // gui_layout.json is not present in the test file operator — construction
    // must not crash when the layout file is missing or empty.
    REQUIRE_NOTHROW(GameRunner(fileOp, renderer));
}

TEST_CASE("GameRunner loads sections from gui_layout.json", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "top_bar":   { "label": "Top Bar",   "x": 0,  "y": 0,  "width": 320, "height": 20  },
            "asset_list":  { "label": "Asset List",  "x": 0,  "y": 20, "width": 64,  "height": 220 },
            "asset_view": { "label": "Asset View", "x": 64, "y": 20, "width": 256, "height": 220 }
        }
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    // asset_list and asset_view are held separately; only top_bar remains as a plain section.
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

    SECTION("asset_list section is created separately")
    {
        REQUIRE(runner.getAssetListSection() != nullptr);
        CHECK(runner.getAssetListSection()->getId() == "asset_list");
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

TEST_CASE("GameRunner loads levels from LEVELS directory", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({
        "name": "Level 1",
        "assets": {
            "/LEVELS/LEVEL_1/asset_1.md":  "Asset 1",
            "/LEVELS/LEVEL_1/asset_2.png": "Asset 2"
        }
    })";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({
        "name": "Level 2",
        "assets": {
            "/LEVELS/LEVEL_2/asset_1.md":  "Asset 1",
            "/LEVELS/LEVEL_2/asset_2.png": "Asset 2"
        }
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getLevels().size() == 2);

    SECTION("level IDs and names are correct")
    {
        const auto& levels = runner.getLevels();
        CHECK(levels[0].id   == "LEVEL_1");
        CHECK(levels[0].name == "Level 1");
        CHECK(levels[1].id   == "LEVEL_2");
        CHECK(levels[1].name == "Level 2");
    }

    SECTION("level assets are loaded with friendly names")
    {
        const auto& assets = runner.getLevels()[0].assets;
        REQUIRE(assets.count("/LEVELS/LEVEL_1/asset_1.md")  == 1);
        REQUIRE(assets.count("/LEVELS/LEVEL_1/asset_2.png") == 1);
        CHECK(assets.at("/LEVELS/LEVEL_1/asset_1.md")  == "Asset 1");
        CHECK(assets.at("/LEVELS/LEVEL_1/asset_2.png") == "Asset 2");
    }
}

TEST_CASE("AssetListSection shows assets for active level", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "asset_list": { "label": "Asset List", "x": 0, "y": 20, "width": 64, "height": 220 }
        }
    })";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{"/LEVELS/LEVEL_1/asset_1.md":"Asset 1","/LEVELS/LEVEL_1/asset_2.png":"Asset 2"}})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2","assets":{"/LEVELS/LEVEL_2/asset_1.md":"Thing A"}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    AssetListSection* list = runner.getAssetListSection();
    REQUIRE(list != nullptr);

    SECTION("initial level assets are loaded")
    {
        REQUIRE(list->getAssets().size() == 2);
    }

    SECTION("assets update when level changes")
    {
        runner.nextLevel();
        REQUIRE(list->getAssets().size() == 1);
        CHECK(list->getAssets()[0] == "Thing A");
    }

    SECTION("selection resets to first asset when level changes")
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

TEST_CASE("GameRunner restores active level and asset from Default_Game_State.json", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"asset_list":{"label":"Asset List","x":0,"y":20,"width":64,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{"/LEVELS/LEVEL_1/asset_1.md":"Asset 1","/LEVELS/LEVEL_1/asset_2.png":"Asset 2"}})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2","assets":{"/LEVELS/LEVEL_2/asset_1.md":"Thing A"}})";
    fileOp.files["/GAME_STATE/Default_Game_State.json"] = R"({"current_level":"Level 2","current_asset":"Thing A","levels":{}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    CHECK(runner.getActiveLevelIndex() == 1);
    REQUIRE(runner.getAssetListSection() != nullptr);
    CHECK(runner.getAssetListSection()->getSelectedName() == "Thing A");
}

TEST_CASE("GameRunner top-bar navigation changes active level", "[GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{}})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2","assets":{}})";

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
