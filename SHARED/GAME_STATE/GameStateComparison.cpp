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

    for (auto it = a.begin(); it != a.end(); ++it)
    {
        if (!it.value().is_object()) continue;

        const std::string& mapKey = it.key();
        if (!b.contains(mapKey) || !b[mapKey].is_object()) continue;

        const auto& mapA = it.value();
        const auto& mapB = b[mapKey];

        for (auto entry = mapA.begin(); entry != mapA.end(); ++entry)
        {
            if (!entry.value().is_boolean()) continue;
            if (!mapB.contains(entry.key())) continue;

            bool before = entry.value().get<bool>();
            bool after  = mapB[entry.key()].get<bool>();

            if (before != after)
                diff.discoveries.push_back({ mapKey, entry.key(), before, after });
        }
    }

    return diff;
}
