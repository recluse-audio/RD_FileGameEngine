#include "ActiveSceneSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

void ActiveSceneSection::setActiveScene(const LevelScene* scene)
{
    mPngPath.clear();
    mMdPath.clear();
    if (!scene) return;
    mPngPath = scene->png;
    mMdPath  = scene->md;
}

void ActiveSceneSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    if (!mPngPath.empty())
        renderer.drawImage(mPngPath, getX(), getY(), getWidth(), getHeight());
    else if (!mMdPath.empty())
        renderer.drawText(mMdPath, getX(), getY());

    renderer.endContentArea();
}
