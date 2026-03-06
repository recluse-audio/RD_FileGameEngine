#include <catch2/catch_test_macros.hpp>
#include "SCENE/Scene.h"

TEST_CASE("Scene getSceneID", "[Scene]")
{
    SECTION("returns the ID passed at construction")
    {
        Scene scene("MyScene");
        REQUIRE(scene.getSceneID() == "MyScene");
    }
}

TEST_CASE("Scene getParentSceneID", "[Scene]")
{
    SECTION("returns the parent ID passed at construction")
    {
        Scene scene("MyScene", "ParentScene");
        REQUIRE(scene.getParentSceneID() == "ParentScene");
    }

    SECTION("defaults to Main when no parent ID is provided")
    {
        Scene scene("MyScene");
        REQUIRE(scene.getParentSceneID() == "Main");
    }
}

TEST_CASE("Scene setSceneID", "[Scene]")
{
    Scene scene("InitialID");

    SECTION("updates the ID")
    {
        scene.setSceneID("UpdatedID");
        REQUIRE(scene.getSceneID() == "UpdatedID");
    }
}

TEST_CASE("Scene addChildScene / getChildScenes", "[Scene]")
{
    Scene scene("MyScene");

    SECTION("starts with no child scenes")
    {
        REQUIRE(scene.getChildScenes().empty());
    }

    SECTION("added child scene appears in list")
    {
        scene.addChildScene("ChildA");
        REQUIRE(scene.getChildScenes().size() == 1);
        REQUIRE(scene.getChildScenes()[0] == "ChildA");
    }

    SECTION("multiple children are all returned")
    {
        scene.addChildScene("ChildA");
        scene.addChildScene("ChildB");
        REQUIRE(scene.getChildScenes().size() == 2);
        REQUIRE(scene.getChildScenes()[1] == "ChildB");
    }
}

TEST_CASE("Scene addZone / getZones", "[Scene]")
{
    Scene scene("MyScene");

    SECTION("starts with no zones")
    {
        REQUIRE(scene.getZones().empty());
    }

    SECTION("added zone appears in list")
    {
        Zone::Bounds bounds(0, 0, 100, 100);
        Zone zone(scene, bounds, "ZoneA");
        scene.addZone(zone);
        REQUIRE(scene.getZones().size() == 1);
        REQUIRE(scene.getZones()[0].getZoneID() == "ZoneA");
    }

    SECTION("multiple zones are all returned")
    {
        Zone::Bounds boundsA(0, 0, 50, 50);
        Zone::Bounds boundsB(60, 60, 50, 50);
        scene.addZone(Zone(scene, boundsA, "ZoneA"));
        scene.addZone(Zone(scene, boundsB, "ZoneB"));
        REQUIRE(scene.getZones().size() == 2);
        REQUIRE(scene.getZones()[1].getZoneID() == "ZoneB");
    }
}
