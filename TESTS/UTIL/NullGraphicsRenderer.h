#pragma once
#include "GRAPHICS_RENDERER/GraphicsRenderer.h"

/** No-op renderer for use in tests that don't exercise drawing. */
class NullGraphicsRenderer : public GraphicsRenderer
{
public:
    void drawImage(const std::string&) override {}
    void drawText(const std::string&, int, int) override {}
    void drawSVG(const std::string&, int, int, int = 0, int = 0) override {}
    void drawButton(const std::string&, int, int, int, int) override {}
};
