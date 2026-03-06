#include <catch2/catch_test_macros.hpp>
#include "GUI_SECTION/GUISection.h"

TEST_CASE("GUISection getLabel", "[GUISection]")
{
    GUISection section("test_section", "Test Section", 0, 0, 100, 100);

    SECTION("returns the label passed at construction")
    {
        REQUIRE(section.getLabel() == "Test Section");
    }
}

// Section: x=10, y=20, w=100, h=50  →  x: [10, 110]  y: [20, 70]
TEST_CASE("GUISection containsPoint", "[GUISection]")
{
    GUISection section("test_section", "Test Section", 10, 20, 100, 50);

    SECTION("point inside returns true")
    {
        REQUIRE(section.containsPoint(50, 40) == true);
    }

    SECTION("point left of bounds returns false")
    {
        REQUIRE(section.containsPoint(9, 40) == false);
    }

    SECTION("point right of bounds returns false")
    {
        REQUIRE(section.containsPoint(111, 40) == false);
    }

    SECTION("point above bounds returns false")
    {
        REQUIRE(section.containsPoint(50, 19) == false);
    }

    SECTION("point below bounds returns false")
    {
        REQUIRE(section.containsPoint(50, 71) == false);
    }

    SECTION("left edge is inclusive")
    {
        REQUIRE(section.containsPoint(10, 40) == true);
    }

    SECTION("right edge is inclusive")
    {
        REQUIRE(section.containsPoint(110, 40) == true);
    }

    SECTION("top edge is inclusive")
    {
        REQUIRE(section.containsPoint(50, 20) == true);
    }

    SECTION("bottom edge is inclusive")
    {
        REQUIRE(section.containsPoint(50, 70) == true);
    }
}
