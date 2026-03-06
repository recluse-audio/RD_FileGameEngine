/**
 * Made by Ryan Devens on 2026-02-26
 */

#pragma once
#include <string>

class GraphicsRenderer;
class Scene;

/**
 * Renders a Scene by dispatching to GraphicsRenderer primitives.
 * Inspects path extensions to choose the correct draw call:
 *   .png  -> drawImage
 *   .md   -> drawText
 *   .svg  -> drawSVG
 *
 * When overlayVisible is true and the scene has a secondaryPath,
 * the secondary asset is drawn as an overlay.
 */
class SceneView
{
public:
    explicit SceneView(GraphicsRenderer& renderer);

    void draw(const Scene& scene, bool overlayVisible, bool zoneDisplayVisible = false);
    void drawMenu(const Scene& menuScene);

private:
    GraphicsRenderer& mRenderer;
};
