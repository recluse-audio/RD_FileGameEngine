#include "ActiveSceneSection.h"
#include "PasswordEntryOverlay.h"
#include "MultipleChoiceOverlay.h"
#include "../GRAPHICS_RENDERER/GraphicsRenderer.h"
#include "../GAME_RUNNER/GameRunner.h"
#include <array>

void ActiveSceneSection::registerHit(int x, int y)
{
    if (mOverlay)
        mOverlay->registerHit(x, y);
}

void ActiveSceneSection::setActiveScene(const LevelScene* scene)
{
    mPngPath.clear();
    mMdPath.clear();
    mZones.clear();
    mOverlay.reset();
    mMdScrollOffset = 0;
    if (!scene) return;

    mPngPath = scene->png;
    mMdPath  = scene->md;
    mZones   = scene->zones;

    if (!scene->isUnlocked && !scene->password.empty())
    {
        mOverlay = std::make_unique<PasswordEntryOverlay>(
            getX(), getY(), getWidth(), getHeight(), scene->password, mGameRunner);
    }
    else if (scene->multipleChoice.size() == MultipleChoiceOverlay::NUM_CHOICES)
    {
        std::array<std::string, MultipleChoiceOverlay::NUM_CHOICES> choices;
        for (int i = 0; i < MultipleChoiceOverlay::NUM_CHOICES; ++i)
            choices[i] = scene->multipleChoice[i];
        mOverlay = std::make_unique<MultipleChoiceOverlay>(
            getX(), getY(), getWidth(), getHeight(), choices, mGameRunner);
    }
}

void ActiveSceneSection::draw(GraphicsRenderer& renderer, bool showLabel) const
{
    GUISection::draw(renderer, showLabel);

    renderer.beginContentArea(getX(), getY(), getWidth(), getHeight());

    if (!mPngPath.empty())
        renderer.drawImage(mPngPath, getX(), getY(), getWidth(), getHeight());
    else if (!mMdPath.empty())
    {
        renderer.setScrollOffset(mMdScrollOffset);
        renderer.drawText(mMdPath, getX(), getY());
        renderer.setScrollOffset(0);
    }

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

    if (mOverlay)
        mOverlay->draw(renderer);
}
