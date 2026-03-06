#include "AssetListSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

void AssetListSection::setAssets(const std::vector<std::string>& names)
{
    mAssetNames    = names;
    mSelectedIndex = names.empty() ? -1 : 0;
}

void AssetListSection::setSelectedIndex(int index)
{
    if (index >= 0 && index < (int)mAssetNames.size())
        mSelectedIndex = index;
}

bool AssetListSection::registerHit(int x, int y)
{
    if (x < getX() || x >= getX() + getWidth()) return false;

    int row = (y - getY() - k_Padding) / k_RowHeight;
    if (row < 0 || row >= (int)mAssetNames.size()) return false;

    mSelectedIndex = row;
    return true;
}

std::string AssetListSection::getSelectedName() const
{
    if (mSelectedIndex < 0 || mSelectedIndex >= (int)mAssetNames.size())
        return "";
    return mAssetNames[mSelectedIndex];
}

void AssetListSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    int y = getY() + k_Padding;
    for (int i = 0; i < (int)mAssetNames.size(); ++i)
    {
        if (y + k_RowHeight > getY() + getHeight()) break;

        if (i == mSelectedIndex)
            renderer.drawFilledRect(getX(), y, getWidth(), k_RowHeight, 255, 255, 255, 60);

        renderer.drawLabel(mAssetNames[i], getX() + k_Padding, y + 3);
        y += k_RowHeight;
    }

    renderer.endContentArea();
}
