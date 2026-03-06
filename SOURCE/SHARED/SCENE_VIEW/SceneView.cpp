#include "SceneView.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "../SCENE/Scene.h"
#include "../ZONE/Zone.h"

static const int CONTENT_X = 0;
static const int CONTENT_Y = 15;
static const int CONTENT_W = 320;
static const int CONTENT_H = 210; // 225 - 15

SceneView::SceneView(GraphicsRenderer& renderer)
: mRenderer(renderer)
{
}

static bool endsWith(const std::string& s, const std::string& suffix)
{
    if (s.size() < suffix.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static void drawPath(GraphicsRenderer& renderer, const std::string& path, int x, int y)
{
    if (path.empty()) return;

    if (endsWith(path, ".png"))
        renderer.drawImage(path);
    else if (endsWith(path, ".md"))
        renderer.drawText(path, x, y);
    else if (endsWith(path, ".svg"))
        renderer.drawSVG(path, x, y);
}

void SceneView::draw(const Scene& scene, bool overlayVisible, bool zoneDisplayVisible)
{
    const std::string& primary = scene.getPrimaryPath();

    if (primary.empty())
    {
        mRenderer.beginContentArea(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H);
        for (auto zone : scene.getZones())
        {
            Zone::Bounds b = zone.getBounds();
            mRenderer.drawButton(zone.getLabel(), b.mX, b.mY, b.mW, b.mH);
        }
        mRenderer.endContentArea();
        return;
    }

    if (endsWith(primary, ".png"))
    {
        drawPath(mRenderer, primary, 0, 0);
    }
    else
    {
        mRenderer.beginContentArea(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H);
        drawPath(mRenderer, primary, 0, 0);
        mRenderer.endContentArea();
    }

    if (overlayVisible)
    {
        mRenderer.beginContentArea(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H);
        drawPath(mRenderer, scene.getSecondaryPath(), 0, 20);
        mRenderer.endContentArea();
    }

    if (zoneDisplayVisible)
    {
        for (auto zone : scene.getZones())
        {
            if (zone.hasPolygon())
                mRenderer.drawPolygon(zone.getPolygon());
            else
            {
                Zone::Bounds b = zone.getBounds();
                mRenderer.drawRect(b.mX, b.mY, b.mW, b.mH);
            }
        }
    }
}

void SceneView::drawMenu(const Scene& menuScene)
{
    mRenderer.beginContentArea(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H);
    for (auto zone : menuScene.getZones())
    {
        Zone::Bounds b = zone.getBounds();
        mRenderer.drawButton(zone.getLabel(), b.mX, b.mY, b.mW, b.mH);
    }
    mRenderer.endContentArea();
}
