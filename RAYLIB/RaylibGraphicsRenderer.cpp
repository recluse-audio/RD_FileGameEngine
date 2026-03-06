#include "RaylibGraphicsRenderer.h"
#include "../../THIRD_PARTY/nanosvg/nanosvg.h"
#include "../../THIRD_PARTY/nanosvg/nanosvgrast.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// ---------------------------------------------------------------------------
// Helpers — all rendering uses 320x240 virtual coordinates.
// gameScale() fits that canvas into the current window while preserving ratio.
// gameOffset() returns the top-left pixel of the letterboxed canvas.
// ---------------------------------------------------------------------------
// GetScreenWidth/Height returns the logical window dimensions, which matches
// the coordinate space used by GetMousePosition() and by Raylib draw calls on
// Windows (the OS handles any DPI scaling transparently when FLAG_WINDOW_HIGHDPI
// is not set). Using render (framebuffer) dimensions here caused a mismatch
// between drawn polygon positions and reported mouse coordinates.
static float gameScale()
{
    return std::min(GetScreenWidth() / 320.0f, GetScreenHeight() / 240.0f);
}

static Vector2 gameOffset()
{
    float s = gameScale();
    return { (GetScreenWidth()  - 320.0f * s) / 2.0f,
             (GetScreenHeight() - 240.0f * s) / 2.0f };
}

// Game-space coord → screen pixel (with letterbox offset)
static float gx(float vx) { return gameOffset().x + vx * gameScale(); }
static float gy(float vy) { return gameOffset().y + vy * gameScale(); }
// Game-space dimension → screen pixels (no offset)
static float gp(float v)  { return v * gameScale(); }

// ---------------------------------------------------------------------------

RaylibGraphicsRenderer::~RaylibGraphicsRenderer()
{
    if (mCachedTexture.id > 0)
        UnloadTexture(mCachedTexture);
    for (auto& [path, tex] : mSvgCache)
        if (tex.id > 0)
            UnloadTexture(tex);
    if (mFont.texture.id > 0)
        UnloadFont(mFont);
}

std::string RaylibGraphicsRenderer::sdPath(const std::string& path)
{
    if (!path.empty() && path[0] == '/')
        return "KSC_DATA" + path;
    return path;
}

void RaylibGraphicsRenderer::toGameCoords(int screenX, int screenY,
                                           int& gameX,  int& gameY) const
{
    float   s   = gameScale();
    Vector2 off = gameOffset();
    gameX = (int)((screenX - off.x) / s);
    gameY = (int)((screenY - off.y) / s);
}

// -----------------------------------------------------------------
void RaylibGraphicsRenderer::drawImage(const std::string& path)
{
    drawPng(sdPath(path));
}

void RaylibGraphicsRenderer::drawText(const std::string& path, int /*x*/, int y)
{
    std::string full = sdPath(path);
    if (y > 0)
    {
        DrawRectangle((int)gx(0), (int)gy(17),
                      (int)gp(320), (int)gp(240 - 17), {0, 0, 0, 200});
        drawMarkdown(full, y, 0.5f, false);
    }
    else
    {
        drawMarkdown(full, 10, 1.0f, true);
    }
}

void RaylibGraphicsRenderer::drawSVG(const std::string& path, int x, int y, int w, int h)
{
    drawSvgAt(sdPath(path), x, y, w, h);
}

void RaylibGraphicsRenderer::drawRect(int x, int y, int w, int h)
{
    DrawRectangleLinesEx({ gx(x), gy(y), gp(w), gp(h) }, 1.0f, { 255, 255, 0, 200 });
}

void RaylibGraphicsRenderer::drawPolygon(const std::vector<std::pair<int, int>>& points)
{
    if (points.size() < 2) return;
    int n = (int)points.size();
    for (int i = 0; i < n; i++)
    {
        int j = (i + 1) % n;
        DrawLine((int)gx(points[i].first), (int)gy(points[i].second),
                 (int)gx(points[j].first), (int)gy(points[j].second),
                 { 255, 255, 0, 200 });
    }
}

void RaylibGraphicsRenderer::beginContentArea(int x, int y, int w, int h)
{
    BeginScissorMode((int)gx(x), (int)gy(y), (int)gp(w), (int)gp(h));
}

void RaylibGraphicsRenderer::endContentArea()
{
    EndScissorMode();
}

void RaylibGraphicsRenderer::drawButton(const std::string& label, int x, int y, int w, int h)
{
    if (mFont.texture.id == 0)
        mFont = LoadFontEx("KSC_DATA/GUI/ASSETS/OcrB2.ttf", 32, nullptr, 0);

    float bx = gx(x), by = gy(y), bw = gp(w), bh = gp(h);
    DrawRectangle((int)bx, (int)by, (int)bw, (int)bh, {0, 0, 0, 220});
    DrawRectangleLines((int)bx, (int)by, (int)bw, (int)bh, WHITE);

    float    fontSize = gp(9);
    Vector2  textSize = MeasureTextEx(mFont, label.c_str(), fontSize, 1.0f);
    DrawTextEx(mFont, label.c_str(),
               { bx + (bw - textSize.x) / 2.0f,
                 by + (bh - textSize.y) / 2.0f },
               fontSize, 1.0f, WHITE);
}

// -----------------------------------------------------------------
void RaylibGraphicsRenderer::drawPng(const std::string& fullPath)
{
    if (fullPath != mCachedPath)
    {
        if (mCachedTexture.id > 0)
            UnloadTexture(mCachedTexture);
        mCachedTexture = LoadTexture(fullPath.c_str());
        mCachedPath    = fullPath;
    }
    if (mCachedTexture.id > 0)
    {
        Rectangle src = { 0, 0,
                          (float)mCachedTexture.width,
                          (float)mCachedTexture.height };
        Vector2 off = gameOffset();
        float   s   = gameScale();
        Rectangle dst = { off.x, off.y, 320.0f * s, 240.0f * s };
        DrawTexturePro(mCachedTexture, src, dst, {0, 0}, 0.0f, WHITE);
    }
}

void RaylibGraphicsRenderer::drawSvgAt(const std::string& fullPath,
                                        int x, int y, int targetW, int targetH)
{
    // Rasterize at 4× for quality; DrawTexturePro scales to current window size.
    static const int RASTER_SCALE = 4;
    std::string cacheKey = fullPath + "@" + std::to_string(targetW)
                                    + "x" + std::to_string(targetH);

    if (mSvgCache.find(cacheKey) == mSvgCache.end())
    {
        std::ifstream file(fullPath);
        if (!file.is_open()) return;
        std::string content((std::istreambuf_iterator<char>(file)), {});

        std::vector<char> buf(content.begin(), content.end());
        buf.push_back('\0');

        NSVGimage* image = nsvgParse(buf.data(), "px", 96.0f);
        if (!image) return;

        float svgW = image->width, svgH = image->height;
        if (svgW <= 0 || svgH <= 0) { nsvgDelete(image); return; }

        int   rasterW = (targetW > 0) ? targetW * RASTER_SCALE : (int)svgW;
        int   rasterH = (targetH > 0) ? targetH * RASTER_SCALE : (int)svgH;
        float scale   = (targetW > 0) ? (float)rasterW / svgW  : 1.0f;

        NSVGrasterizer* rast = nsvgCreateRasterizer();
        std::vector<uint8_t> pixels(rasterW * rasterH * 4);
        nsvgRasterize(rast, image, 0, 0, scale,
                      pixels.data(), rasterW, rasterH, rasterW * 4);
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);

        for (size_t i = 0; i < pixels.size(); i += 4)
            pixels[i] = pixels[i+1] = pixels[i+2] = 255;

        Image img = { pixels.data(), rasterW, rasterH, 1,
                      PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
        mSvgCache[cacheKey] = LoadTextureFromImage(img);
    }

    const Texture2D& tex = mSvgCache.at(cacheKey);
    if (tex.id <= 0) return;

    Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
    Rectangle dst = { gx(x), gy(y), gp(targetW), gp(targetH) };
    DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
}

void RaylibGraphicsRenderer::drawMarkdown(const std::string& fullPath,
                                           int startY, float textScale,
                                           bool applyScroll)
{
    std::ifstream file(fullPath);
    if (!file.is_open()) return;

    if (mFont.texture.id == 0)
        mFont = LoadFontEx("KSC_DATA/GUI/ASSETS/OcrB2.ttf", 32, nullptr, 0);

    float contentTop    = gy(15);
    float contentBottom = gy(225);
    float yPos = applyScroll ? contentTop - (float)mScrollOffset
                             : gy(startY);

    std::string line;
    while (std::getline(file, line))
    {
        int hashes = 0;
        while (hashes < (int)line.size() && line[hashes] == '#') hashes++;

        std::string display = (hashes > 0 && hashes < (int)line.size())
                              ? line.substr(hashes + 1)
                              : line;

        float fontSize = gp((float)((hashes == 1) ? 14 : (hashes == 2) ? 11 : 9)
                            * textScale);
        float lineH    = fontSize + gp(4 * textScale);
        Color color    = (hashes > 0) ? WHITE : LIGHTGRAY;

        if (yPos + lineH > contentTop && yPos < contentBottom)
            DrawTextEx(mFont, display.c_str(), { gx(10), yPos },
                       fontSize, 1.0f, color);

        yPos += lineH;
    }
}
