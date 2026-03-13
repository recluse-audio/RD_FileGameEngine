#include "PasswordEntryOverlay.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "../GAME_RUNNER/GameRunner.h"

PasswordEntryOverlay::PasswordEntryOverlay(int secX, int secY, int secW, int secH,
                                           const std::string& password, GameRunner* gr)
    : mPassword(password)
    , mGameRunner(gr)
    , mSecX(secX), mSecY(secY), mSecW(secW), mSecH(secH)
{
    constexpr int boxSize   = 24;
    constexpr int boxGap    = 6;
    constexpr int chevronH  = 12;
    constexpr int entryH    = chevronH + boxSize + chevronH;
    constexpr int submitW   = 40;
    constexpr int submitH   = 16;
    constexpr int submitGap = 6;
    constexpr int skipW     = 30; // TESTING ONLY
    constexpr int btnGap    = 4;  // TESTING ONLY

    const int numBoxes   = (int)mPassword.size();
    const int totalW     = numBoxes * boxSize + (numBoxes - 1) * boxGap;
    const int startX     = secX + (secW - totalW) / 2;
    const int topY       = secY + secH / 4 + 8;

    for (int i = 0; i < numBoxes; ++i)
        mPasswordEntry.addBox({ startX + i * (boxSize + boxGap), topY, boxSize, chevronH });

    const int totalBtnsW = submitW + btnGap + skipW; // TESTING ONLY
    mSubmitBtnX = secX + (secW - totalBtnsW) / 2;
    mSubmitBtnY = topY + entryH + submitGap;
    mSubmitBtnW = submitW;
    mSubmitBtnH = submitH;

    // TESTING ONLY - skip button bypasses password check
    mSkipBtnX = mSubmitBtnX + submitW + btnGap;
    mSkipBtnY = mSubmitBtnY;
    mSkipBtnW = skipW;
    mSkipBtnH = submitH;
}

void PasswordEntryOverlay::draw(GraphicsRenderer& renderer) const
{
    renderer.drawFilledRect(mSecX, mSecY, mSecW, mSecH, 0, 0, 0, 255);
    renderer.drawCenteredLabel("LOCKED", mSecX, mSecY, mSecW, mSecH / 4);

    mPasswordEntry.draw(renderer);
    if (mSubmitBtnW > 0)
        renderer.drawButton("ENTER", mSubmitBtnX, mSubmitBtnY, mSubmitBtnW, mSubmitBtnH);
    if (mSkipBtnW > 0) // TESTING ONLY
        renderer.drawButton("SKIP", mSkipBtnX, mSkipBtnY, mSkipBtnW, mSkipBtnH);
}

void PasswordEntryOverlay::registerHit(int x, int y)
{
    if (mSubmitBtnW > 0
        && x >= mSubmitBtnX && x < mSubmitBtnX + mSubmitBtnW
        && y >= mSubmitBtnY && y < mSubmitBtnY + mSubmitBtnH)
    {
        if (mGameRunner)
            mGameRunner->submitPassword(mPasswordEntry.getEntryString());
        return;
    }

    // TESTING ONLY - submits correct password directly, remove before final product
    if (mSkipBtnW > 0
        && x >= mSkipBtnX && x < mSkipBtnX + mSkipBtnW
        && y >= mSkipBtnY && y < mSkipBtnY + mSkipBtnH)
    {
        if (mGameRunner)
            mGameRunner->submitPassword(mPassword);
        return;
    }

    mPasswordEntry.registerHit(x, y);
}
