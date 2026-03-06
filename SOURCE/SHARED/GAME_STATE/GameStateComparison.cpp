#include "GameStateComparison.h"
#include <nlohmann/json.hpp>

static const std::vector<std::string> k_ScalarFields = {
    "currentMode", "currentLocation", "currentNote"
};

GameStateComparison::GameStateComparison(const std::string& jsonA, const std::string& jsonB)
: mJsonA(jsonA)
, mJsonB(jsonB)
{
}

bool GameStateComparison::isEqual() const
{
    return mJsonA == mJsonB;
}

static void collectBoolDiffs(const nlohmann::json& a, const nlohmann::json& b,
                              const std::string& parentPath,
                              GameStateComparison::Diff& diff)
{
    for (auto it = a.begin(); it != a.end(); ++it)
    {
        if (!b.contains(it.key())) continue;

        if (it.value().is_boolean())
        {
            bool before = it.value().get<bool>();
            bool after  = b[it.key()].get<bool>();
            if (before != after)
                diff.discoveries.push_back({ parentPath, it.key(), before, after });
        }
        else if (it.value().is_object() && b[it.key()].is_object())
        {
            std::string childPath = parentPath.empty() ? it.key() : parentPath + "." + it.key();
            collectBoolDiffs(it.value(), b[it.key()], childPath, diff);
        }
    }
}

GameStateComparison::Diff GameStateComparison::getDiff() const
{
    Diff diff;

    nlohmann::json a = nlohmann::json::parse(mJsonA, nullptr, false);
    nlohmann::json b = nlohmann::json::parse(mJsonB, nullptr, false);

    if (a.is_discarded() || b.is_discarded())
        return diff;

    for (const std::string& field : k_ScalarFields)
    {
        std::string va = a.value(field, "");
        std::string vb = b.value(field, "");
        if (va != vb)
            diff.scalars.push_back({ field, va, vb });
    }

    collectBoolDiffs(a, b, "", diff);

    return diff;
}
