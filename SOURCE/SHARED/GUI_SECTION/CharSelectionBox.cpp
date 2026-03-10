#include "CharSelectionBox.h"

CharSelectionBox::CharSelectionBox(int x, int y, int boxSize, int chevronH)
: mX(x), mY(y), mBoxSize(boxSize), mChevronH(chevronH)
{}

char CharSelectionBox::getValue() const
{
    return k_Chars[mIndex];
}

void CharSelectionBox::increment()
{
    mIndex = (mIndex + 1) % k_NumChars;
}

void CharSelectionBox::decrement()
{
    mIndex = (mIndex - 1 + k_NumChars) % k_NumChars;
}

bool CharSelectionBox::registerHit(int x, int y)
{
    if (x < mX || x >= mX + mBoxSize)
        return false;

    const int upTop    = mY;
    const int upBot    = mY + mChevronH;
    const int downTop  = mY + mChevronH + mBoxSize;
    const int downBot  = downTop + mChevronH;

    if (y >= upTop && y < upBot)   { increment(); return true; }
    if (y >= downTop && y < downBot) { decrement(); return true; }
    return false;
}

void CharSelectionBox::draw(GraphicsRenderer& renderer) const
{
    const int upY   = mY;
    const int boxY  = mY + mChevronH;
    const int downY = boxY + mBoxSize;

    renderer.drawCenteredLabel("^", mX, upY,   mBoxSize, mChevronH);
    renderer.drawRect(mX, boxY, mBoxSize, mBoxSize);
    renderer.drawCenteredLabel(std::string(1, getValue()), mX, boxY, mBoxSize, mBoxSize);
    renderer.drawCenteredLabel("v", mX, downY, mBoxSize, mChevronH);
}
