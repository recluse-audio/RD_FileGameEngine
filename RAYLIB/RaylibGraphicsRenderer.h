/**
 * Made by Ryan Devens on 2026-02-26
 */

#pragma once
#include "../SHARED/GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "raylib.h"
#include <string>
#include <unordered_map>

/**
 * Raylib (desktop) implementation of GraphicsRenderer.
 *
 * drawImage  — loads a PNG as a Raylib texture and blits it at the origin.
 *              The most recently loaded texture is cached.
 * drawText   — reads a markdown file line by line and renders it as plain text.
 *              When y > 0 (overlay mode) a semi-transparent backdrop is drawn
 *              before the text and the text is rendered at reduced scale.
 * drawSVG    — rasterizes an SVG via nanosvg and draws it at (x, y).
 *              Rasterized textures are cached by path.
 */
class RaylibGraphicsRenderer : public GraphicsRenderer
{
public:
    ~RaylibGraphicsRenderer();

    void drawImage(const std::string& path) override;
    void drawText(const std::string& path, int x, int y) override;
    void drawSVG(const std::string& path, int x, int y, int w = 0, int h = 0) override;
    void drawButton(const std::string& label, int x, int y, int w, int h) override;
    void drawRect(int x, int y, int w, int h) override;
    void drawPolygon(const std::vector<std::pair<int, int>>& points) override;
    void beginContentArea(int x, int y, int w, int h) override;
    void endContentArea() override;

    /** Convert a screen-space pixel position to 320x240 game coordinates. */
    void toGameCoords(int screenX, int screenY, int& gameX, int& gameY) const;

private:
    std::string mCachedPath;
    Texture2D   mCachedTexture = {};
    std::unordered_map<std::string, Texture2D> mSvgCache;
    Font        mFont          = {};

    static std::string sdPath(const std::string& path);
    void drawPng(const std::string& fullPath);
    void drawMarkdown(const std::string& fullPath, int startY = 10, float textScale = 1.0f, bool applyScroll = false);
    void drawSvgAt(const std::string& fullPath, int x, int y, int targetW = 0, int targetH = 0);
};
