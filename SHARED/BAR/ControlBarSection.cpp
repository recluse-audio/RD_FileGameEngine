#include "ControlBarSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include <nlohmann/json.hpp>

ControlBarSection::ControlBarSection(GraphicsRenderer& renderer)
: mRenderer(renderer)
{
}

void ControlBarSection::load(const std::string& json)
{
    mButtons.clear();
    auto j = nlohmann::json::parse(json, nullptr, false);
    if (j.is_discarded()) return;

    for (auto& entry : j["buttons"])
    {
        Button b;
        b.id          = entry.value("id",          "");
        b.x           = entry.value("x",           0);
        b.y           = entry.value("y",           0);
        b.w           = entry.value("w",           0);
        b.h           = entry.value("h",           0);
        b.icon        = entry.value("icon",        "");
        b.callback    = entry.value("callback",    "");
        b.visibleWhen = entry.value("visibleWhen", "always");
        mButtons.push_back(b);
    }
}

void ControlBarSection::setState(const BarState& state)
{
    mState = state;
}

bool ControlBarSection::isButtonVisible(const Button& btn) const
{
    if (btn.visibleWhen == "always")    return true;
    if (btn.visibleWhen == "root")      return mState.isRoot;
    if (btn.visibleWhen == "nonRoot")   return !mState.isRoot;
    if (btn.visibleWhen == "notesMode")        return mState.mode == "notes";
    if (btn.visibleWhen == "locationsNonRoot") return mState.mode != "notes" && !mState.isRoot;
    return false;
}

void ControlBarSection::draw()
{
    for (const auto& btn : mButtons)
    {
        if (isButtonVisible(btn) && !btn.icon.empty())
            mRenderer.drawSVG(btn.icon, btn.x, btn.y, btn.w, btn.h);
    }
}

std::string ControlBarSection::handleHit(int x, int y) const
{
    for (const auto& btn : mButtons)
    {
        if (!isButtonVisible(btn)) continue;
        if (x >= btn.x && x <= btn.x + btn.w &&
            y >= btn.y && y <= btn.y + btn.h)
            return btn.callback;
    }
    return "";
}
