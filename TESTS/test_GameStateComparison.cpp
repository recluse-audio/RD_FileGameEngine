#include <catch2/catch_test_macros.hpp>
#include "GAME_STATE/GameStateComparison.h"
#include "UTIL/TestFileOperator.h"
#include <nlohmann/json.hpp>

// Flip every boolean leaf in a JSON object to true, recursively.
static nlohmann::json withAllTrue(nlohmann::json j)
{
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (it.value().is_boolean())
            it.value() = true;
        else if (it.value().is_object())
            it.value() = withAllTrue(it.value());
    }
    return j;
}

TEST_CASE("GameStateComparison against default game state", "[GameStateComparison]")
{
    TestFileOperator fileOp;
    fileOp.diskRoot = "DATA";

    std::string defaultJson = fileOp.load("/GAME_STATE/Default_Game_State.json");
    REQUIRE_FALSE(defaultJson.empty());

    nlohmann::json defaultDoc = nlohmann::json::parse(defaultJson);
    nlohmann::json completeDoc = withAllTrue(defaultDoc);
    std::string completeJson = completeDoc.dump(2);

    GameStateComparison cmp(defaultJson, completeJson);

    SECTION("default and complete states are not identical")
    {
        REQUIRE_FALSE(cmp.isEqual());
    }

    SECTION("identical states report equal")
    {
        GameStateComparison same(defaultJson, defaultJson);
        REQUIRE(same.isEqual());
        REQUIRE(same.getDiff().isEmpty());
    }

    SECTION("no scalar field differences")
    {
        auto diff = cmp.getDiff();
        REQUIRE(diff.scalars.empty());
    }

    SECTION("all discoveries are false -> true")
    {
        auto diff = cmp.getDiff();
        REQUIRE_FALSE(diff.discoveries.empty());
        for (const auto& change : diff.discoveries)
        {
            CHECK(change.before == false);
            CHECK(change.after  == true);
        }
    }
}
