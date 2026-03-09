/**
 * Made by Ryan Devens on 2026-03-09
 */

#pragma once
#include "RaylibGameLibrary.h"
#include "RaylibGraphicsRenderer.h"
#include "raylib.h"
#include <algorithm>
#include <string>
#include <vector>

/**
 * Full-screen game-selection UI rendered with raw Raylib calls.
 * Runs its own event loop until the user picks a game or closes the window.
 * Returns the selected game's dataPath, or "" if the window was closed with
 * no selection.
 *
 * Coordinate space: 320×240 logical pixels, scaled to match the window.
 */
class GamePickerScreen
{
public:
    explicit GamePickerScreen(const std::vector<GameEntry>& games)
        : mGames(games) {}

    std::string run()
    {
        while (!WindowShouldClose())
        {
            handleInput();

            BeginDrawing();
            ClearBackground(BLACK);
            draw();
            EndDrawing();

            if (mConfirmedIndex >= 0)
                return mGames[mConfirmedIndex].dataPath;
        }
        return "";
    }

private:
    const std::vector<GameEntry>& mGames;
    int  mHoverIndex     = -1;
    int  mSelectedIndex  = 0;
    int  mConfirmedIndex = -1;

    static constexpr int k_ItemX = 20;
    static constexpr int k_ItemW = 280;
    static constexpr int k_ItemH = 20;
    static constexpr int k_ListY = 40;
    static constexpr int k_ItemSpacing = 24;

    float scale() const
    {
        return (float)GetScreenWidth() / 320.0f;
    }

    int itemCount() const { return (int)mGames.size(); }

    Rectangle itemRect(int i) const
    {
        float s = scale();
        float y = (float)(k_ListY + i * k_ItemSpacing);
        return { k_ItemX * s, y * s, k_ItemW * s, k_ItemH * s };
    }

    void handleInput()
    {
        if (itemCount() == 0) return;

        if (IsKeyPressed(KEY_DOWN))
            mSelectedIndex = (mSelectedIndex + 1) % itemCount();
        if (IsKeyPressed(KEY_UP))
            mSelectedIndex = (mSelectedIndex - 1 + itemCount()) % itemCount();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            mConfirmedIndex = mSelectedIndex;
            return;
        }

        Vector2 mouse = GetMousePosition();
        mHoverIndex = -1;
        for (int i = 0; i < itemCount(); ++i)
        {
            Rectangle r = itemRect(i);
            if (CheckCollisionPointRec(mouse, r))
            {
                mHoverIndex = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    mConfirmedIndex = i;
            }
        }
    }

    void draw()
    {
        float s = scale();
        int   fs = std::max(6, (int)(8 * s));

        DrawText("SELECT A GAME", (int)(20 * s), (int)(16 * s), fs, WHITE);

        if (itemCount() == 0)
        {
            DrawText("No games found.", (int)(20 * s), (int)(50 * s), fs, RED);
            return;
        }

        for (int i = 0; i < itemCount(); ++i)
        {
            Rectangle r = itemRect(i);
            Color bg;
            if (i == mConfirmedIndex)
                bg = { 100, 200, 100, 255 };
            else if (i == mSelectedIndex)
                bg = {  60, 120, 200, 255 };
            else if (i == mHoverIndex)
                bg = {  50,  50,  70, 255 };
            else
                bg = {  30,  30,  30, 255 };

            DrawRectangleRec(r, bg);
            DrawText(mGames[i].name.c_str(),
                     (int)(r.x + 6 * s),
                     (int)(r.y + 4 * s),
                     fs, WHITE);
        }

        DrawText("UP/DOWN + ENTER  or  click to select",
                 (int)(20 * s), (int)(220 * s),
                 std::max(4, (int)(6 * s)), GRAY);
    }
};
