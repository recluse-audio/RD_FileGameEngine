#include "ActiveSceneSection.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "../GAME_RUNNER/GameRunner.h"

void ActiveSceneSection::registerHit(int x, int y)
{
    if (mSubmitBtnW > 0
        && x >= mSubmitBtnX && x < mSubmitBtnX + mSubmitBtnW
        && y >= mSubmitBtnY && y < mSubmitBtnY + mSubmitBtnH)
    {
        if (mGameRunner)
            mGameRunner->submitPassword(mPasswordEntry.getEntryString());
        return;
    }

    mPasswordEntry.registerHit(x, y);
}

void ActiveSceneSection::setActiveScene(const LevelScene* scene)
{
    mPngPath.clear();
    mMdPath.clear();
    mZones.clear();
    mPassword.clear();
    mPasswordEntry.clear();
    mIsLocked = false;
    if (!scene) return;
    mPngPath  = scene->png;
    mMdPath   = scene->md;
    mZones    = scene->zones;
    mIsLocked = !scene->isUnlocked;
    mPassword = scene->password;

    mSubmitBtnX = mSubmitBtnY = mSubmitBtnW = mSubmitBtnH = 0;

    if (mIsLocked && !mPassword.empty())
    {
        constexpr int boxSize   = 12;
        constexpr int boxGap    = 4;
        constexpr int chevronH  = 6;
        constexpr int entryH    = chevronH + boxSize + chevronH;
        constexpr int submitW   = 40;
        constexpr int submitH   = 12;
        constexpr int submitGap = 6;
        const int numBoxes      = (int)mPassword.size();
        const int totalW        = numBoxes * boxSize + (numBoxes - 1) * boxGap;
        const int startX        = getX() + (getWidth() - totalW) / 2;
        const int topY          = getY() + getHeight() / 4 + 8;
        for (int i = 0; i < numBoxes; ++i)
            mPasswordEntry.addBox({ startX + i * (boxSize + boxGap), topY, boxSize, chevronH });

        mSubmitBtnX = getX() + (getWidth() - submitW) / 2;
        mSubmitBtnY = topY + entryH + submitGap;
        mSubmitBtnW = submitW;
        mSubmitBtnH = submitH;
    }
}

void ActiveSceneSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    if (!mPngPath.empty())
        renderer.drawImage(mPngPath, getX(), getY(), getWidth(), getHeight());
    else if (!mMdPath.empty())
        renderer.drawText(mMdPath, getX(), getY());

    if (mShowZones)
    {
        for (const auto& zone : mZones)
        {
            if (!zone.points.empty())
                renderer.drawPolygon(zone.points);
            else
                renderer.drawRect(zone.x, zone.y, zone.w, zone.h);

            int lx = zone.points.empty() ? zone.x : zone.points[0].first;
            int ly = zone.points.empty() ? zone.y : zone.points[0].second;
            renderer.drawLabel(zone.id, lx, ly);
        }
    }

    renderer.endContentArea();

    if (mShowOverlay)
    {
        renderer.drawFilledRect(getX(), getY(), getWidth(), getHeight(), 0, 0, 0, 255);
        renderer.drawLabel("Overlay", getX() + 4, getY() + 4);
    }

    if (mIsLocked)
    {
        renderer.drawFilledRect(getX(), getY(), getWidth(), getHeight(), 0, 0, 0, 180);
        renderer.drawCenteredLabel("LOCKED", getX(), getY(), getWidth(), getHeight() / 4);

        mPasswordEntry.draw(renderer);
        if (mSubmitBtnW > 0)
            renderer.drawButton("ENTER", mSubmitBtnX, mSubmitBtnY, mSubmitBtnW, mSubmitBtnH);
    }
}
