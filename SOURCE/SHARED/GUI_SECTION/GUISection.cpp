#include "GUISection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

GUISection::GUISection(std::string id, std::string label, int x, int y, int width, int height)
: mId(std::move(id))
, mLabel(std::move(label))
, mX(x)
, mY(y)
, mWidth(width)
, mHeight(height)
{
}

void GUISection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    renderer.drawFilledRect(mX, mY, mWidth, mHeight, mR, mG, mB, 128);
    if (showLabel)
        renderer.drawLabel(mLabel, mX + 4, mY + 4);
}

bool GUISection::containsPoint(int x, int y) const
{
    return x >= mX && x <= mX + mWidth &&
           y >= mY && y <= mY + mHeight;
}
