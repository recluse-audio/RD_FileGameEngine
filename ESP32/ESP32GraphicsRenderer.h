/**
 * Made by Ryan Devens on 2026-02-26
 */

#pragma once
#include "../SHARED/GRAPHICS_RENDERER/GraphicsRenderer.h"

#include <TFT_eSPI.h>
#include <string>

/**
 * ESP32 implementation of GraphicsRenderer.
 * Uses TFT_eSPI for display output and PNGdec for PNG decoding.
 * drawSVG is a no-op (SVG rendering not supported on ESP32).
 */
class ESP32GraphicsRenderer : public GraphicsRenderer
{
public:
    explicit ESP32GraphicsRenderer(TFT_eSPI& tft);

    void drawImage(const std::string& path) override;
    void drawText(const std::string& path, int x, int y) override;
    void drawSVG(const std::string& path, int x, int y, int w = 0, int h = 0) override;
    void drawButton(const std::string& label, int x, int y, int w, int h) override;

private:
    TFT_eSPI& mTft;
};
