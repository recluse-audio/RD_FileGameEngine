/**
 * Made by Ryan Devens on 2026-03-09
 */

#pragma once
#include "GameFolderBrowser.h"
#include "RaylibGameLibrary.h"
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
 * Coordinate space: 320x240 logical pixels, scaled to match the window.
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

            if (!mBrowsedPath.empty())
                return mBrowsedPath;
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
    bool mFileMenuOpen   = false;
    std::string mBrowsedPath;

    static constexpr int k_MenuH        = 14;
    static constexpr int k_TitleY       = k_MenuH + 6;
    static constexpr int k_ListY        = k_TitleY + 18;
    static constexpr int k_ItemX        = 20;
    static constexpr int k_ItemW        = 280;
    static constexpr int k_ItemH        = 20;
    static constexpr int k_ItemSpacing  = 24;
    static constexpr int k_FileMenuX    = 2;
    static constexpr int k_FileMenuBtnW = 36;
    static constexpr int k_DropdownW    = 110;
    static constexpr int k_DropdownH    = 14;

    float scale() const { return (float)GetScreenWidth() / 320.0f; }
    int   itemCount() const { return (int)mGames.size(); }

    Rectangle menuBarRect() const
    {
        float s = scale();
        return { 0, 0, 320 * s, (float)(k_MenuH * s) };
    }

    Rectangle fileButtonRect() const
    {
        float s = scale();
        return { (float)(k_FileMenuX * s), s,
                 (float)(k_FileMenuBtnW * s), (float)((k_MenuH - 2) * s) };
    }

    Rectangle openFolderItemRect() const
    {
        float s = scale();
        return { (float)(k_FileMenuX * s), (float)(k_MenuH * s),
                 (float)(k_DropdownW * s), (float)(k_DropdownH * s) };
    }

    Rectangle itemRect(int i) const
    {
        float s = scale();
        return { (float)(k_ItemX * s),
                 (float)((k_ListY + i * k_ItemSpacing) * s),
                 (float)(k_ItemW * s),
                 (float)(k_ItemH * s) };
    }

    void handleInput()
    {
        Vector2 mouse = GetMousePosition();

        if (mFileMenuOpen)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (CheckCollisionPointRec(mouse, openFolderItemRect()))
                {
                    mFileMenuOpen = false;
                    mBrowsedPath  = browseForGameFolder();
                    return;
                }
                if (!CheckCollisionPointRec(mouse, fileButtonRect()))
                    mFileMenuOpen = false;
            }
            return;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(mouse, fileButtonRect()))
        {
            mFileMenuOpen = true;
            return;
        }

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
        float s   = scale();
        int   fs  = std::max(6, (int)(8 * s));
        int   mfs = std::max(4, (int)(7 * s));
        int   sfs = std::max(4, (int)(6 * s));

        // ---- menu bar ----
        DrawRectangleRec(menuBarRect(), { 28, 28, 38, 255 });

        Rectangle fb  = fileButtonRect();
        Color     fbg = mFileMenuOpen ? Color{ 70, 70, 100, 255 }
                                      : Color{ 28, 28,  38, 255 };
        DrawRectangleRec(fb, fbg);
        DrawText("FILE", (int)(fb.x + 3 * s), (int)(fb.y + 2 * s), mfs, WHITE);

        if (mFileMenuOpen)
        {
            Rectangle drop = openFolderItemRect();
            DrawRectangleRec(drop, { 45, 45, 65, 255 });
            DrawRectangleLinesEx(drop, 1, { 90, 90, 110, 255 });
            DrawText("Open Game Folder...",
                     (int)(drop.x + 4 * s), (int)(drop.y + 2 * s), mfs, WHITE);
        }

        // ---- title ----
        DrawText("SELECT A GAME", (int)(20 * s), (int)(k_TitleY * s), fs, WHITE);

        // ---- game list ----
        if (itemCount() == 0)
        {
            DrawText("No games found in:",
                     (int)(20 * s), (int)(k_ListY * s), fs, RED);
            DrawText(RaylibGameLibrary::k_DefaultRoot,
                     (int)(20 * s), (int)((k_ListY + 20) * s), sfs, GRAY);
            DrawText("Use  File > Open Game Folder  to browse.",
                     (int)(20 * s), (int)((k_ListY + 40) * s), sfs, GRAY);
            return;
        }

        for (int i = 0; i < itemCount(); ++i)
        {
            Rectangle r = itemRect(i);
            Color bg;
            if      (i == mConfirmedIndex) bg = { 100, 200, 100, 255 };
            else if (i == mSelectedIndex)  bg = {  60, 120, 200, 255 };
            else if (i == mHoverIndex)     bg = {  50,  50,  70, 255 };
            else                           bg = {  30,  30,  30, 255 };

            DrawRectangleRec(r, bg);
            DrawText(mGames[i].name.c_str(),
                     (int)(r.x + 6 * s), (int)(r.y + 4 * s), fs, WHITE);
        }

        DrawText("UP/DOWN + ENTER  or  click  |  File > Open Game Folder",
                 (int)(20 * s), (int)(220 * s), sfs, GRAY);
    }
};
