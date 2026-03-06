/**
 * Made by Ryan Devens on 2026-03-06
 */

#pragma once
#include <string>

class GraphicsRenderer;

/**
 * Represents a named rectangular area of the 320x240 app window.
 * Sections are defined in DATA/GUI/gui_layout.json and loaded at startup.
 */
class GUISection
{
public:
    GUISection(std::string id, std::string label, int x, int y, int width, int height);

    virtual void draw(GraphicsRenderer& renderer, bool showLabel = false) const;
    bool containsPoint(int x, int y) const;

    void setColor(int r, int g, int b) { mR = r; mG = g; mB = b; }

    std::string getId()     const { return mId; }
    std::string getLabel()  const { return mLabel; }
    int         getX()      const { return mX; }
    int         getY()      const { return mY; }
    int         getWidth()  const { return mWidth; }
    int         getHeight() const { return mHeight; }

private:
    std::string mId;
    std::string mLabel;
    int         mX      = 0;
    int         mY      = 0;
    int         mWidth  = 0;
    int         mHeight = 0;
    int         mR      = 128;
    int         mG      = 128;
    int         mB      = 128;
};
