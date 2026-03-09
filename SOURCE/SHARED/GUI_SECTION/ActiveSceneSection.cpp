#include "ActiveSceneSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

void ActiveSceneSection::setActiveScene(const LevelScene* scene)
{
    mPngPath.clear();
    mMdPath.clear();
    mZones.clear();
    if (!scene) return;
    mPngPath = scene->png;
    mMdPath  = scene->md;
    mZones   = scene->zones;
}

void ActiveSceneSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    if (!mPngPath.empty())
        renderer.drawImage(mPngPath, getX(), getY(), getWidth(), getHeight());
    else if (!mMdPath.empty())
        renderer.drawText(mMdPath, getX(), getY());

    if (mShowZones)
    {
        for (const auto& zone : mZones)
        {
            if (!zone.points.empty())
                renderer.drawPolygon(zone.points);
            else
                renderer.drawRect(zone.x, zone.y, zone.w, zone.h);

            int lx = zone.points.empty() ? zone.x : zone.points[0].first;
            int ly = zone.points.empty() ? zone.y : zone.points[0].second;
            renderer.drawLabel(zone.id, lx, ly);
        }
    }

    renderer.endContentArea();
}
