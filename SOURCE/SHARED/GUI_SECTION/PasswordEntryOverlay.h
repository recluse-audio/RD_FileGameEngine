/**
 * Made by Ryan Devens on 2026-03-12
 */

#pragma once
#include "ActiveSceneOverlay.h"
#include "PasswordEntryComponent.h"
#include <string>

class GameRunner;

/**
 * Overlay for scenes locked behind a password.
 *
 * Draws a full black cover over the scene content, a "LOCKED" label, the
 * character-scroll entry boxes, and ENTER / SKIP buttons.
 *
 * Geometry is computed in the constructor from the owning section's bounds.
 */
class PasswordEntryOverlay : public ActiveSceneOverlay
{
public:
    PasswordEntryOverlay(int secX, int secY, int secW, int secH,
                         const std::string& password, GameRunner* gr);

    void draw(GraphicsRenderer& renderer) const override;
    void registerHit(int x, int y) override;

private:
    std::string            mPassword;
    PasswordEntryComponent mPasswordEntry;
    GameRunner*            mGameRunner  = nullptr;
    int                    mSecX        = 0;
    int                    mSecY        = 0;
    int                    mSecW        = 0;
    int                    mSecH        = 0;
    int                    mSubmitBtnX  = 0;
    int                    mSubmitBtnY  = 0;
    int                    mSubmitBtnW  = 0;
    int                    mSubmitBtnH  = 0;
    // TESTING ONLY - remove before final product
    int                    mSkipBtnX    = 0;
    int                    mSkipBtnY    = 0;
    int                    mSkipBtnW    = 0;
    int                    mSkipBtnH    = 0;
};
