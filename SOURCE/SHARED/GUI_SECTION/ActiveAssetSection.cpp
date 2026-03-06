#include "ActiveAssetSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

static bool endsWith(const std::string& s, const std::string& suffix)
{
    if (s.size() < suffix.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// A path with no extension in its last component is an asset folder
// (e.g. /LEVELS/LEVEL_1/avery_1) containing platform-specific image variants.
static bool isAssetFolder(const std::string& path)
{
    auto slash = path.rfind('/');
    const std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    return name.find('.') == std::string::npos;
}

void ActiveAssetSection::setLevelAssets(const std::map<std::string, std::string>& assets)
{
    mLevelAssets = assets;
    mActivePath.clear();
}

void ActiveAssetSection::setActiveAsset(const std::string& friendlyName)
{
    mActivePath.clear();
    if (friendlyName.empty()) return;

    for (const auto& [path, friendly] : mLevelAssets)
    {
        if (friendly == friendlyName)
        {
            mActivePath = path;
            return;
        }
    }
}

void ActiveAssetSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);
    if (mActivePath.empty()) return;

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    if (endsWith(mActivePath, ".png") || isAssetFolder(mActivePath))
        renderer.drawImage(mActivePath, getX(), getY(), getWidth(), getHeight());
    else if (endsWith(mActivePath, ".svg"))
        renderer.drawSVG(mActivePath, getX(), getY(), getWidth(), getHeight());
    else
        renderer.drawText(mActivePath, getX(), getY());

    renderer.endContentArea();
}
