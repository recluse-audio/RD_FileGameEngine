#include "SceneListSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

void SceneListSection::setScenes(const std::vector<std::string>& names)
{
    mSceneNames    = names;
    mSelectedIndex = names.empty() ? -1 : 0;
}

void SceneListSection::setSelectedIndex(int index)
{
    if (index >= 0 && index < (int)mSceneNames.size())
        mSelectedIndex = index;
}

bool SceneListSection::registerHit(int x, int y)
{
    if (x < getX() || x >= getX() + getWidth()) return false;

    int row = (y - getY() - k_Padding) / k_RowHeight;
    if (row < 0 || row >= (int)mSceneNames.size()) return false;

    mSelectedIndex = row;
    return true;
}

std::string SceneListSection::getSelectedName() const
{
    if (mSelectedIndex < 0 || mSelectedIndex >= (int)mSceneNames.size())
        return "";
    return mSceneNames[mSelectedIndex];
}

void SceneListSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    int y = getY() + k_Padding;
    for (int i = 0; i < (int)mSceneNames.size(); ++i)
    {
        if (y + k_RowHeight > getY() + getHeight()) break;

        if (i == mSelectedIndex)
            renderer.drawFilledRect(getX(), y, getWidth(), k_RowHeight, 255, 255, 255, 60);

        renderer.drawLabel(mSceneNames[i], getX() + k_Padding, y + 3);
        y += k_RowHeight;
    }

    renderer.endContentArea();
}
