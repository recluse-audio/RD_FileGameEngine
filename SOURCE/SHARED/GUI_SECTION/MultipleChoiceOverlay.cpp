#include "MultipleChoiceOverlay.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"

MultipleChoiceOverlay::MultipleChoiceOverlay(int secX, int secY, int secW, int secH,
                                             const std::array<std::string, NUM_CHOICES>& choices,
                                             GameRunner* gr)
    : mGameRunner(gr)
{
    constexpr int pad  = 6;
    constexpr int btnH = 32;
    const int     btnW = (secW - 3 * pad) / 2;

    // 2×2 grid anchored to the bottom of the section
    const int gridH = 2 * btnH + 3 * pad;
    const int gridY = secY + secH - gridH;

    const int col0X = secX + pad;
    const int col1X = secX + 2 * pad + btnW;
    const int row0Y = gridY + pad;
    const int row1Y = gridY + 2 * pad + btnH;

    mChoices[0] = { choices[0], col0X, row0Y, btnW, btnH };
    mChoices[1] = { choices[1], col1X, row0Y, btnW, btnH };
    mChoices[2] = { choices[2], col0X, row1Y, btnW, btnH };
    mChoices[3] = { choices[3], col1X, row1Y, btnW, btnH };
}

void MultipleChoiceOverlay::draw(GraphicsRenderer& renderer) const
{
    for (const auto& btn : mChoices)
        renderer.drawButton(btn.label, btn.x, btn.y, btn.w, btn.h);
}

void MultipleChoiceOverlay::registerHit(int x, int y)
{
    for (int i = 0; i < NUM_CHOICES; ++i)
    {
        const auto& btn = mChoices[i];
        if (x >= btn.x && x < btn.x + btn.w
            && y >= btn.y && y < btn.y + btn.h)
        {
            // TODO: wire up to GameRunner::submitMultipleChoice(i) when test logic is ready
            return;
        }
    }
}
