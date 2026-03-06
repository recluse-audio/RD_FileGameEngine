/**
 * Made by Ryan Devens on 2026-03-03
 */

#pragma once
#include <string>
#include <vector>

class GraphicsRenderer;

/**
 * Snapshot of game state that ControlBarSection uses to decide which buttons
 * are visible.
 */
struct BarState
{
    bool        isRoot         = false;
    bool        overlayVisible = false;
    bool        hasParent      = false;
    std::string mode           = "locations";
};

/**
 * Renders a persistent navigation bar (top or bottom) and resolves hit tests
 * against its buttons. Loaded from a JSON data file via load().
 *
 * draw() iterates visible buttons and calls mRenderer.drawSVG() for each icon.
 * handleHit() returns the callback identifier for the button at (x, y), or ""
 * if no visible button was hit.
 */
class ControlBarSection
{
public:
    explicit ControlBarSection(GraphicsRenderer& renderer);

    void        load(const std::string& json);
    void        setState(const BarState& state);
    void        draw();
    std::string handleHit(int x, int y) const;

private:
    struct Button
    {
        std::string id;
        int         x           = 0;
        int         y           = 0;
        int         w           = 0;
        int         h           = 0;
        std::string icon;
        std::string callback;
        std::string visibleWhen; // "always", "root", "nonRoot", "notesMode", "locationsNonRoot"
    };

    bool isButtonVisible(const Button& btn) const;

    GraphicsRenderer&   mRenderer;
    std::vector<Button> mButtons;
    BarState            mState;
};
