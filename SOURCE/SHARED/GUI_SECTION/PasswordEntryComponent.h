/**
 * Made by Ryan Devens on 2026-03-09
 */

#pragma once
#include "CharSelectionBox.h"
#include <string>
#include <vector>

class GraphicsRenderer;

/**
 * Manages an ordered row of CharSelectionBoxes for password entry.
 *
 * Boxes are stored and drawn left to right in the order they were added.
 * getEntryString() returns the current character from each box concatenated
 * in that same order.
 */
class PasswordEntryComponent
{
public:
    PasswordEntryComponent() = default;

    void addBox(CharSelectionBox box) { mBoxes.push_back(std::move(box)); }
    void clear()                      { mBoxes.clear(); }
    bool empty() const                { return mBoxes.empty(); }

    /** Concatenates the current character of each box, left to right. */
    std::string getEntryString() const;

    /** Forwards the hit to the first box that claims it. */
    bool registerHit(int x, int y);

    void draw(GraphicsRenderer& renderer) const;

private:
    std::vector<CharSelectionBox> mBoxes;
};
