#include <catch2/catch_test_macros.hpp>
#include "GUI_SECTION/ActiveSceneSection.h"
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

    void drawImage(const std::string& path) override                           { lastImagePath = path; }
    void drawImage(const std::string& path, int, int, int, int) override       { lastImagePath = path; }
    void drawText(const std::string& path, int, int) override                  { lastTextPath  = path; }
    void drawSVG(const std::string& path, int, int, int = 0, int = 0) override { lastSvgPath   = path; }
    void drawButton(const std::string&, int, int, int, int) override           {}
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static LevelScene makeScene(const std::string& id, const std::string& name,
                             const std::string& md = "", const std::string& png = "")
{
    LevelScene s;
    s.id   = id;
    s.name = name;
    s.md   = md;
    s.png  = png;
    return s;
}

// ---------------------------------------------------------------------------
// setActiveScene — path storage
// ---------------------------------------------------------------------------

TEST_CASE("ActiveSceneSection stores paths from scene", "[ActiveSceneSection]")
{
    ActiveSceneSection section("scene_view", "Scene View", 64, 20, 256, 220);

    SECTION("scene with only md")
    {
        auto s = makeScene("SCENE_1", "Intro", "/LEVELS/LEVEL_1/SCENE_1/intro.md");
        section.setActiveScene(&s);
        CHECK(section.getMdPath()  == "/LEVELS/LEVEL_1/SCENE_1/intro.md");
        CHECK(section.getPngPath() == "");
    }

    SECTION("scene with only png")
    {
        auto s = makeScene("SCENE_1", "Banner", "", "/LEVELS/LEVEL_1/SCENE_1/banner.png");
        section.setActiveScene(&s);
        CHECK(section.getPngPath() == "/LEVELS/LEVEL_1/SCENE_1/banner.png");
        CHECK(section.getMdPath()  == "");
    }

    SECTION("scene with both md and png")
    {
        auto s = makeScene("SCENE_1", "Scene",
                           "/LEVELS/LEVEL_1/SCENE_1/notes.md",
                           "/LEVELS/LEVEL_1/SCENE_1/scene.png");
        section.setActiveScene(&s);
        CHECK(section.getPngPath() == "/LEVELS/LEVEL_1/SCENE_1/scene.png");
        CHECK(section.getMdPath()  == "/LEVELS/LEVEL_1/SCENE_1/notes.md");
    }

    SECTION("nullptr clears both paths")
    {
        auto s = makeScene("SCENE_1", "Intro", "/LEVELS/LEVEL_1/SCENE_1/intro.md");
        section.setActiveScene(&s);
        REQUIRE_FALSE(section.getMdPath().empty());

        section.setActiveScene(nullptr);
        CHECK(section.getPngPath().empty());
        CHECK(section.getMdPath().empty());
    }
}

// ---------------------------------------------------------------------------
// draw — renderer dispatch
// ---------------------------------------------------------------------------

TEST_CASE("ActiveSceneSection draw dispatches by content type", "[ActiveSceneSection]")
{
    ActiveSceneSection section("scene_view", "Scene View", 64, 20, 256, 220);
    SpyGraphicsRenderer spy;

    SECTION("md-only scene calls drawText")
    {
        auto s = makeScene("SCENE_1", "Intro", "/LEVELS/LEVEL_1/SCENE_1/intro.md");
        section.setActiveScene(&s);
        section.draw(spy);
        CHECK(spy.lastTextPath  == "/LEVELS/LEVEL_1/SCENE_1/intro.md");
        CHECK(spy.lastImagePath.empty());
    }

    SECTION("png-only scene calls drawImage")
    {
        auto s = makeScene("SCENE_1", "Banner", "", "/LEVELS/LEVEL_1/SCENE_1/banner.png");
        section.setActiveScene(&s);
        section.draw(spy);
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/SCENE_1/banner.png");
        CHECK(spy.lastTextPath.empty());
    }

    SECTION("scene with both png and md calls drawImage (png takes priority)")
    {
        auto s = makeScene("SCENE_1", "Scene",
                           "/LEVELS/LEVEL_1/SCENE_1/notes.md",
                           "/LEVELS/LEVEL_1/SCENE_1/scene.png");
        section.setActiveScene(&s);
        section.draw(spy);
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/SCENE_1/scene.png");
        CHECK(spy.lastTextPath.empty());
    }

    SECTION("no active scene draws nothing")
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

TEST_CASE("GameRunner creates ActiveSceneSection for scene_view", "[ActiveSceneSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({
        "sections": {
            "scene_list": { "label": "Scene List", "x": 0,  "y": 20, "width": 64,  "height": 220 },
            "scene_view": { "label": "Scene View", "x": 64, "y": 20, "width": 256, "height": 220 }
        }
    })";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"First Scene","png":"scene.png"})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveSceneSection() != nullptr);
    CHECK(runner.getActiveSceneSection()->getId() == "scene_view");
}

TEST_CASE("GameRunner ActiveSceneSection reflects initial state from Default_Game_State.json", "[ActiveSceneSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"scene_list":{"label":"Scene List","x":0,"y":20,"width":64,"height":220},"scene_view":{"label":"Scene View","x":64,"y":20,"width":256,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"First Scene","png":"banner.png"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_2/scene_info.json"] = R"({"name":"Second Scene","md":"notes.md"})";
    fileOp.files["/GAME_STATE/Default_Game_State.json"] = R"({"current_level":"Level 1","current_scene":"Second Scene","levels":{}})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveSceneSection() != nullptr);
    CHECK(runner.getActiveSceneSection()->getMdPath()  == "/LEVELS/LEVEL_1/SCENE_2/notes.md");
    CHECK(runner.getActiveSceneSection()->getPngPath() == "");
}

TEST_CASE("GameRunner ActiveSceneSection updates when level changes", "[ActiveSceneSection][GameRunner]")
{
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"scene_list":{"label":"Scene List","x":0,"y":20,"width":64,"height":220},"scene_view":{"label":"Scene View","x":64,"y":20,"width":256,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"Intro","md":"intro.md"})";
    fileOp.files["/LEVELS/LEVEL_2/level_info.json"] = R"({"name":"Level 2"})";
    fileOp.files["/LEVELS/LEVEL_2/SCENE_1/scene_info.json"] = R"({"name":"Readme","md":"readme.md"})";

    NullGraphicsRenderer renderer;
    GameRunner runner(fileOp, renderer);

    REQUIRE(runner.getActiveSceneSection() != nullptr);
    CHECK(runner.getActiveSceneSection()->getMdPath() == "/LEVELS/LEVEL_1/SCENE_1/intro.md");

    runner.nextLevel();
    CHECK(runner.getActiveSceneSection()->getMdPath() == "/LEVELS/LEVEL_2/SCENE_1/readme.md");
}

TEST_CASE("registerHit on scene list updates ActiveSceneSection", "[ActiveSceneSection][GameRunner]")
{
    // Layout: scene_list x=0 y=20 w=64 h=220, k_Padding=4, k_RowHeight=14.
    // Row N centre y = 20 + 4 + N*14 + 7.
    TestFileOperator fileOp;
    fileOp.files["/GUI/gui_layout.json"] = R"({"sections":{"scene_list":{"label":"Scene List","x":0,"y":20,"width":64,"height":220},"scene_view":{"label":"Scene View","x":64,"y":20,"width":256,"height":220}}})";
    fileOp.files["/LEVELS/LEVEL_1/level_info.json"] = R"({"name":"Level 1"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_1/scene_info.json"] = R"({"name":"With Image","png":"scene.png"})";
    fileOp.files["/LEVELS/LEVEL_1/SCENE_2/scene_info.json"] = R"({"name":"With Text","md":"notes.md"})";

    SpyGraphicsRenderer spy;
    GameRunner runner(fileOp, spy);

    REQUIRE(runner.getActiveSceneSection() != nullptr);

    SECTION("initial selection (row 0) draws png via drawImage")
    {
        runner.draw();
        CHECK(spy.lastImagePath == "/LEVELS/LEVEL_1/SCENE_1/scene.png");
    }

    SECTION("clicking row 1 switches to md scene drawn via drawText")
    {
        runner.registerHit(32, 45);  // row 1 centre
        CHECK(runner.getActiveSceneSection()->getMdPath() == "/LEVELS/LEVEL_1/SCENE_2/notes.md");
        runner.draw();
        CHECK(spy.lastTextPath == "/LEVELS/LEVEL_1/SCENE_2/notes.md");
    }
}
