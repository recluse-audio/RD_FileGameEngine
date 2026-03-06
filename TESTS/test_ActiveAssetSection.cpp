#include <catch2/catch_test_macros.hpp>
#include "GUI_SECTION/ActiveAssetSection.h"
#include "GAME_RUNNER/GameRunner.h"
#include "UTIL/TestFileOperator.h"
#include "UTIL/NullGraphicsRenderer.h"

// ---------------------------------------------------------------------------
// Spy renderer — records which draw method was last called and with what path.
// ---------------------------------------------------------------------------
class SpyGraphicsRenderer : public GraphicsRenderer
{
public:
    std::string lastImagePath;
    std::string lastTextPath;
    std::string lastSvgPath;

    void drawImage(const std::string& path) override              { lastImagePath = path; }
    void drawImage(const std::string& path, int, int, int, int) override { lastImagePath = path; }
    void drawText(const std::string& path, int, int) override     { lastTextPath  = path; }
    void drawSVG(const std::string& path, int, int, int = 0, int = 0) override { lastSvgPath = path; }
    void drawButton(const std::string&, int, int, int, int) override {}
};

// ---------------------------------------------------------------------------
// Helper — a minimal asset map matching the level_info.json convention.
// ---------------------------------------------------------------------------
static std::map<std::string, std::string> makeAssets()
{
    return {
        { "/LEVELS/LEVEL_1/intro.md",    "Intro"    },
        { "/LEVELS/LEVEL_1/banner.png",  "Banner"   },
        { "/LEVELS/LEVEL_1/diagram.svg", "Diagram"  },
    };
}

// ---------------------------------------------------------------------------
// setLevelAssets / setActiveAsset — path resolution
// ---------------------------------------------------------------------------

TEST_CASE("ActiveAssetSection resolves friendly name to path", "[ActiveAssetSection]")
{
    ActiveAssetSection section("asset_view", "Asset View", 64, 20, 256, 220);
    section.setLevelAssets(makeAssets());

    SECTION("known friendly name resolves to the matching path")
    {
        section.setActiveAsset("Intro");
        CHECK(section.getActivePath() == "/LEVELS/LEVEL_1/intro.md");

        section.setActiveAsset("Banner");
        CHECK(section.getActivePath() == "/LEVELS/LEVEL_1/banner.png");

        section.setActiveAsset("Diagram");
        CHECK(section.getActivePath() == "/LEVELS/LEVEL_1/diagram.svg");
    }

    SECTION("unknown friendly name leaves path empty")
    {
        section.setActiveAsset("DoesNotExist");
        CHECK(section.getActivePath().empty());
    }

    SECTION("empty friendly name leaves path empty")
    {
        section.setActiveAsset("Intro");
        section.setActiveAsset("");
        CHECK(section.getActivePath().empty());
    }
}

TEST_CASE("ActiveAssetSection setLevelAssets clears the active path", "[ActiveAssetSection]")
{
    ActiveAssetSection section("asset_view", "Asset View", 64, 20, 256, 220);
    section.setLevelAssets(makeAssets());
    section.setActiveAsset("Intro");
    REQUIRE_FALSE(section.getActivePath().empty());

    section.setLevelAssets({});
    CHECK(section.getActivePath().empty());
}

// ---------------------------------------------------------------------------
// draw — renderer dispatch by file extension
// ---------------------------------------------------------------------------

TEST_CASE("ActiveAssetSection draw dispatches by file extension", "[ActiveAssetSection]")
{
    ActiveAssetSection  section("asset_view", "Asset View", 64, 20, 256, 220);
    SpyGraphicsRenderer spy;
    section.setLevelAssets(makeAssets());

    SECTION(".md asset calls drawText")
    {
        section.setActiveAsset("Intro");
        section.draw(spy);
        CHECK(spy.lastTextPath  == "/LEVELS/LEVEL_1/intro.md");
        CHECK(spy.lastImagePath.empty());
        CHECK(spy.lastSvgPath.empty());
    }

    SECTION(".png asset calls drawImage")
    {
        section.setActiveAsset("Banner");
        section.draw(spy);
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/banner.png");
        CHECK(spy.lastTextPath.empty());
        CHECK(spy.lastSvgPath.empty());
    }

    SECTION(".svg asset calls drawSVG")
    {
        section.setActiveAsset("Diagram");
        section.draw(spy);
        CHECK(spy.lastSvgPath  == "/LEVELS/LEVEL_1/diagram.svg");
        CHECK(spy.lastImagePath.empty());
        CHECK(spy.lastTextPath.empty());
    }

    SECTION("no active asset draws nothing beyond background")
    {
        section.draw(spy);
        CHECK(spy.lastImagePath.empty());
        CHECK(spy.lastTextPath.empty());
        CHECK(spy.lastSvgPath.empty());
    }
}

// ---------------------------------------------------------------------------
// GameRunner integration — wiring
// ---------------------------------------------------------------------------

TEST_CASE("GameRunner creates ActiveAssetSection for asset_view", "[ActiveAssetSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "asset_list": { "label": "Asset List", "x": 0,  "y": 20, "width": 64,  "height": 220 },
            "asset_view": { "label": "Asset View", "x": 64, "y": 20, "width": 256, "height": 220 }
        }
    })";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({
        "name": "Level 1",
        "assets": {
            "/LEVELS/LEVEL_1/intro.md":   "Intro",
            "/LEVELS/LEVEL_1/banner.png": "Banner"
        }
    })";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveAssetSection() != nullptr);
    CHECK(runner.getActiveAssetSection()->getId() == "asset_view");
}

TEST_CASE("GameRunner ActiveAssetSection reflects initial state from Default_Game_State.json", "[ActiveAssetSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"asset_list":{"label":"Asset List","x":0,"y":20,"width":64,"height":220},"asset_view":{"label":"Asset View","x":64,"y":20,"width":256,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{"/LEVELS/LEVEL_1/intro.md":"Intro","/LEVELS/LEVEL_1/banner.png":"Banner"}})";
    fileOp.files["/GAME_STATE/Default_Game_State.json"] = R"({"current_level":"Level 1","current_asset":"Banner","levels":{}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveAssetSection() != nullptr);
    CHECK(runner.getActiveAssetSection()->getActivePath() == "/LEVELS/LEVEL_1/banner.png");
}

TEST_CASE("GameRunner ActiveAssetSection updates when level changes", "[ActiveAssetSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"asset_list":{"label":"Asset List","x":0,"y":20,"width":64,"height":220},"asset_view":{"label":"Asset View","x":64,"y":20,"width":256,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{"/LEVELS/LEVEL_1/intro.md":"Intro"}})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2","assets":{"/LEVELS/LEVEL_2/readme.md":"Readme"}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveAssetSection() != nullptr);
    CHECK(runner.getActiveAssetSection()->getActivePath() == "/LEVELS/LEVEL_1/intro.md");

    runner.nextLevel();
    CHECK(runner.getActiveAssetSection()->getActivePath() == "/LEVELS/LEVEL_2/readme.md");
}

TEST_CASE("ActiveAssetSection treats extension-less path as asset folder image", "[ActiveAssetSection]")
{
    std::map<std::string, std::string> assets = {
        { "/LEVELS/LEVEL_1/avery_1",       "Avery"  },
        { "/LEVELS/LEVEL_1/intro.md",       "Intro"  },
    };

    ActiveAssetSection section("asset_view", "Asset View", 64, 20, 256, 220);
    section.setLevelAssets(assets);

    SECTION("folder path resolves correctly")
    {
        section.setActiveAsset("Avery");
        CHECK(section.getActivePath() == "/LEVELS/LEVEL_1/avery_1");
    }

    SECTION("draw dispatches folder path to drawImage")
    {
        SpyGraphicsRenderer spy;
        section.setActiveAsset("Avery");
        section.draw(spy);
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/avery_1");
        CHECK(spy.lastTextPath.empty());
        CHECK(spy.lastSvgPath.empty());
    }

    SECTION("draw dispatches plain .md path to drawText (not affected)")
    {
        SpyGraphicsRenderer spy;
        section.setActiveAsset("Intro");
        section.draw(spy);
        CHECK(spy.lastTextPath  == "/LEVELS/LEVEL_1/intro.md");
        CHECK(spy.lastImagePath.empty());
    }
}

TEST_CASE("registerHit on asset list updates ActiveAssetSection draw dispatch", "[ActiveAssetSection][GameRunner]")
{
    // Layout: asset_list x=0 y=20 w=64 h=220, asset_view x=64 y=20 w=256 h=220.
    // k_Padding=4, k_RowHeight=14.
    // Assets sorted by path: avery_1 (row 0), banner.png (row 1), intro.md (row 2).
    // Row N y-centre = 20 + 4 + N*14 + 7.
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"asset_list":{"label":"Asset List","x":0,"y":20,"width":64,"height":220},"asset_view":{"label":"Asset View","x":64,"y":20,"width":256,"height":220}}})" ;
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1","assets":{"/LEVELS/LEVEL_1/avery_1":"Avery","/LEVELS/LEVEL_1/banner.png":"Banner","/LEVELS/LEVEL_1/intro.md":"Intro"}})";

    SpyGraphicsRenderer spy;
    GameRunner runner(fileOp, spy);

    REQUIRE(runner.getActiveAssetSection() != nullptr);

    SECTION("initial selection (row 0) draws asset folder via drawImage")
    {
        // avery_1 is an asset folder � routed to drawImage
        runner.draw();
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/avery_1");
    }

    SECTION("clicking row 1 switches to banner.png drawn via drawImage")
    {
        runner.registerHit(32, 45);  // row 1 centre
        CHECK(runner.getActiveAssetSection()->getActivePath() == "/LEVELS/LEVEL_1/banner.png");
        runner.draw();
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/banner.png");
    }

    SECTION("clicking row 2 switches to intro.md drawn via drawText")
    {
        runner.registerHit(32, 59);  // row 2 centre
        CHECK(runner.getActiveAssetSection()->getActivePath() == "/LEVELS/LEVEL_1/intro.md");
        runner.draw();
        CHECK(spy.lastTextPath == "/LEVELS/LEVEL_1/intro.md");
    }
}
