#include <catch2/catch_test_macros.hpp>
#include "SCENE/Scene.h"

// Bounds: x=10, y=20, w=100, h=50  →  x: [10, 110]  y: [20, 70]
TEST_CASE("Zone containsPoint", "[Zone]")
{
    Scene scene("TestScene");
    Zone::Bounds bounds(10, 20, 100, 50);
    Zone zone(scene, bounds, "TestZone");

    SECTION("point inside returns true")
    {
        REQUIRE(zone.containsPoint(50, 40) == true);
    }

    SECTION("point left of bounds returns false")
    {
        REQUIRE(zone.containsPoint(9, 40) == false);
    }

    SECTION("point right of bounds returns false")
    {
        REQUIRE(zone.containsPoint(111, 40) == false);
    }

    SECTION("point above bounds returns false")
    {
        REQUIRE(zone.containsPoint(50, 19) == false);
    }

    SECTION("point below bounds returns false")
    {
        REQUIRE(zone.containsPoint(50, 71) == false);
    }

    SECTION("left edge is inclusive")
    {
        REQUIRE(zone.containsPoint(10, 40) == true);
    }

    SECTION("right edge is inclusive")
    {
        REQUIRE(zone.containsPoint(110, 40) == true);
    }

    SECTION("top edge is inclusive")
    {
        REQUIRE(zone.containsPoint(50, 20) == true);
    }

    SECTION("bottom edge is inclusive")
    {
        REQUIRE(zone.containsPoint(50, 70) == true);
    }
}

// Triangle polygon: (10,10), (50,10), (30,40)
TEST_CASE("Zone polygon containsPoint", "[Zone]")
{
    Scene scene("TestScene");
    Zone::Bounds bounds(10, 10, 40, 30);
    Zone zone(scene, bounds, "PolyZone");
    zone.setPolygon({{ 10, 10 }, { 50, 10 }, { 30, 40 }});

    SECTION("centroid is inside")
    {
        REQUIRE(zone.containsPoint(30, 20) == true);
    }

    SECTION("point outside triangle but inside bounding rect returns false")
    {
        REQUIRE(zone.containsPoint(12, 38) == false);
    }

    SECTION("point outside bounding rect returns false")
    {
        REQUIRE(zone.containsPoint(5, 20) == false);
    }

    SECTION("vertex is inside")
    {
        REQUIRE(zone.containsPoint(10, 10) == true);
    }
}

TEST_CASE("Zone hasPolygon", "[Zone]")
{
    Scene scene("TestScene");
    Zone::Bounds bounds(0, 0, 100, 100);
    Zone zone(scene, bounds, "Z");

    SECTION("false before setPolygon")
    {
        REQUIRE(zone.hasPolygon() == false);
    }

    SECTION("true after setPolygon")
    {
        zone.setPolygon({{ 0, 0 }, { 10, 0 }, { 5, 10 }});
        REQUIRE(zone.hasPolygon() == true);
    }
}

TEST_CASE("Zone getZoneID", "[Zone]")
{
    Scene scene("TestScene");
    Zone::Bounds bounds(0, 0, 100, 100);

    SECTION("returns the ID passed at construction")
    {
        Zone zone(scene, bounds, "MyZone");
        REQUIRE(zone.getZoneID() == "MyZone");
    }

    SECTION("returns default ID when none provided")
    {
        Zone zone(scene, bounds);
        REQUIRE(zone.getZoneID() == "Default Zone ID");
    }
}
