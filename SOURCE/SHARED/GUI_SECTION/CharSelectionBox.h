/**
 * Made by Ryan Devens on 2026-03-09
 */

#pragma once
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

/**
 * A single character-selection widget for password entry.
 *
 * Displays a rectangle containing the current character, with an up chevron
 * above and a down chevron below. Clicking the chevron areas cycles through
 * the character set: a-z then 0-9 (36 characters), wrapping at both ends.
 *
 * Layout (top to bottom):
 *   [chevronH] up chevron "^"
 *   [boxSize]  character rect
 *   [chevronH] down chevron "v"
 */
class CharSelectionBox
{
public:
    CharSelectionBox(int x, int y, int boxSize, int chevronH);

    char getValue() const;
    void increment();
    void decrement();

    /** Returns true if the hit landed on an up or down chevron and was handled. */
    bool registerHit(int x, int y);

    void draw(GraphicsRenderer& renderer) const;

    int totalHeight() const { return mChevronH + mBoxSize + mChevronH; }

private:
    static constexpr const char* k_Chars    = "abcdefghijklmnopqrstuvwxyz0123456789";
    static constexpr int         k_NumChars = 36;

    int mX;
    int mY;
    int mBoxSize;
    int mChevronH;
    int mIndex = 0;
};
