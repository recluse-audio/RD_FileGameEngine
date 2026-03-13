/**
 * Made by Ryan Devens on 2026-03-12
 */

#pragma once
#include "ActiveSceneOverlay.h"
#include <array>
#include <string>

class GameRunner;

/**
 * A single labelled button within the multiple-choice overlay.
 */
struct ChoiceButton
{
    std::string label;
    int x = 0, y = 0, w = 0, h = 0;
};

/**
 * Overlay for scenes with a "multipleChoice" array in scene_info.json.
 *
 * Renders four labelled buttons in a 2×2 grid over the lower half of the
 * section. The scene content (png or md question body) remains visible behind
 * the buttons.
 *
 * Test logic (answer checking, state transitions) is not wired up yet.
 */
class MultipleChoiceOverlay : public ActiveSceneOverlay
{
public:
    static constexpr int NUM_CHOICES = 4;

    MultipleChoiceOverlay(int secX, int secY, int secW, int secH,
                          const std::array<std::string, NUM_CHOICES>& choices,
                          GameRunner* gr);

    void draw(GraphicsRenderer& renderer) const override;
    void registerHit(int x, int y) override;

private:
    std::array<ChoiceButton, NUM_CHOICES> mChoices;
    GameRunner*                           mGameRunner = nullptr;
};
