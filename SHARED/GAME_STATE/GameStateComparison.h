/**
 * Made by Ryan Devens on 2026-02-27
 */

#pragma once
#include <string>
#include <vector>

/**
 * Compares two Game_State.json documents.
 *
 * Usage:
 *   GameStateComparison cmp(jsonA, jsonB);
 *   if (!cmp.isEqual()) { auto diff = cmp.getDiff(); ... }
 */
class GameStateComparison
{
public:
    struct ScalarChange
    {
        std::string field;  // e.g. "currentMode"
        std::string before;
        std::string after;
    };

    struct DiscoveryChange
    {
        std::string mapKey;   // e.g. "avery_locations"
        std::string path;     // scene path key
        bool        before;
        bool        after;
    };

    struct Diff
    {
        std::vector<ScalarChange>    scalars;
        std::vector<DiscoveryChange> discoveries;

        bool isEmpty() const { return scalars.empty() && discoveries.empty(); }
    };

    GameStateComparison(const std::string& jsonA, const std::string& jsonB);

    /** Returns true if both documents are identical. */
    bool isEqual() const;

    /** Returns a breakdown of every difference between the two documents. */
    Diff getDiff() const;

private:
    std::string mJsonA;
    std::string mJsonB;
};
