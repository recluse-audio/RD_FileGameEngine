#include <catch2/catch_test_macros.hpp>
#include "GAME_RUNNER/GameStartManager.h"
#include "GAME_STATE/GameStateComparison.h"
#include "UTIL/TestFileOperator.h"
#include <algorithm>
#include <filesystem>

/**
 * These tests are intended to cover the creation of a new save slot
 * (aka new game state folder is made in the KSC_DATA)
 *
 * For tests of switching to the correct start location use test_GameRunner.cpp
 */
namespace fs = std::filesystem;

static const std::string k_GoldenPath      = "/GAME_STATE/Game_State.json";
static const std::string k_CompletePath    = "TESTS/GOLDEN/GAME_STATE/Game_State_Complete.json";
static const std::string k_OutputDir       = "TESTS/OUTPUT/GAME_START_MANAGER/KSC_DATA";
static const std::string k_AveryNotePath   = "/GAME_STATE/NOTES_STATE/AVERY/Avery_Note.md";
static const std::string k_LibraryNotePath = "/GAME_STATE/NOTES_STATE/LIBRARY/Library_Note.md";

// Clears the output dir once for the entire test run so slots accumulate cleanly.
static bool s_outputReady = false;
static void prepareOutputDir()
{
    if (!s_outputReady)
    {
        fs::remove_all(k_OutputDir);
        s_outputReady = true;
    }
    fs::create_directories(k_OutputDir);
}

// Runs save() and returns the slot directory that was written.
// Always loads Game_State.json and the full NOTES_STATE into the in-memory map.
static std::string runSave(TestFileOperator& fileOp)
{
    prepareOutputDir();

    fileOp.diskRoot = "../KSC/KSC_DATA";

    for (const std::string& path : { k_GoldenPath, k_AveryNotePath, k_LibraryNotePath })
    {
        std::string content = fileOp.load(path);
        REQUIRE_FALSE(content.empty());
        fileOp.files[path] = content;
    }

    // Pre-compute the slot that GameStartManager will choose.
    int nextSlot = 0;
    while (fs::exists(k_OutputDir + "/KSC_SLOT_" + std::to_string(nextSlot)))
        nextSlot++;

    GameStartManager manager(fileOp, k_OutputDir);
    manager.save();

    return k_OutputDir + "/KSC_SLOT_" + std::to_string(nextSlot);
}

TEST_CASE("GameStartManager setSaveDir updates the save directory", "[GameStartManager]")
{
    TestFileOperator fileOp;
    const std::string defaultDir = "/KSC_GAME/SAVED_GAMES";

    GameStartManager manager(fileOp, defaultDir);
    REQUIRE(manager.getSaveDir() == defaultDir);

    manager.setSaveDir(k_OutputDir);
    REQUIRE(manager.getSaveDir() == k_OutputDir);
}

TEST_CASE("GameStartManager save copies golden Game_State.json into slot", "[GameStartManager]")
{
    TestFileOperator fileOp;
    std::string slotDir   = runSave(fileOp);
    std::string savedJson = fileOp.load(slotDir + "/Game_State.json");

    REQUIRE(fs::is_directory(slotDir));
    REQUIRE_FALSE(savedJson.empty());

    std::string goldenJson = fileOp.load(k_GoldenPath);
    GameStateComparison cmp(goldenJson, savedJson);
    REQUIRE(cmp.isEqual());
}

TEST_CASE("GameStartManager save slot is not yet the complete game state", "[GameStartManager]")
{
    TestFileOperator fileOp;
    std::string slotDir    = runSave(fileOp);
    std::string savedJson  = fileOp.load(slotDir + "/Game_State.json");
    std::string completeJson = fileOp.load(k_CompletePath);

    REQUIRE_FALSE(savedJson.empty());
    REQUIRE_FALSE(completeJson.empty());

    GameStateComparison cmp(savedJson, completeJson);

    SECTION("save slot differs from complete state")
    {
        REQUIRE_FALSE(cmp.isEqual());
    }

    SECTION("no scalar differences between save slot and complete state")
    {
        auto diff = cmp.getDiff();
        REQUIRE(diff.scalars.empty());
    }

    SECTION("save slot has five undiscovered clues vs complete state")
    {
        auto diff = cmp.getDiff();
        REQUIRE(diff.discoveries.size() == 5);
    }

    SECTION("every undiscovered entry is false in save slot and true in complete state")
    {
        auto diff = cmp.getDiff();
        for (const auto& change : diff.discoveries)
        {
            CHECK(change.mapKey == "avery_locations");
            CHECK(change.before == false);
            CHECK(change.after  == true);
        }
    }
}

TEST_CASE("GameStartManager save copies NOTES_STATE into slot", "[GameStartManager]")
{
    TestFileOperator fileOp;
    std::string slotDir = runSave(fileOp);

    SECTION("NOTES_STATE/AVERY subdirectory exists")
    {
        REQUIRE(fs::is_directory(slotDir + "/NOTES_STATE/AVERY"));
    }

    SECTION("Avery_Note.md content matches original")
    {
        std::string written  = fileOp.load(slotDir + "/NOTES_STATE/AVERY/Avery_Note.md");
        std::string original = fileOp.load("../KSC/KSC_DATA/GAME_STATE/NOTES_STATE/AVERY/Avery_Note.md");
        REQUIRE_FALSE(written.empty());
        REQUIRE(written == original);
    }

    SECTION("Library_Note.md content matches original")
    {
        std::string written  = fileOp.load(slotDir + "/NOTES_STATE/LIBRARY/Library_Note.md");
        std::string original = fileOp.load("../KSC/KSC_DATA/GAME_STATE/NOTES_STATE/LIBRARY/Library_Note.md");
        REQUIRE_FALSE(written.empty());
        REQUIRE(written == original);
    }
}
