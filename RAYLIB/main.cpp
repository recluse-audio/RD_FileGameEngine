/**
 * KSC — Raylib desktop entry point
 * Made by Ryan Devens on 2026-02-25
 */

#include "raylib.h"
#include "RaylibFileOperator.h"
#include "RaylibGraphicsRenderer.h"
#include "../SHARED/GAME_RUNNER/GameRunner.h"
#include <filesystem>

static const int SCALE    = 2;
static const int SCREEN_W = 320 * SCALE;
static const int SCREEN_H = 240 * SCALE;

int main()
{
    // Walk up from the exe directory until we find a folder containing KSC_DATA/.
    // This lets the exe run from any working directory.
    {
        std::filesystem::path dir = GetApplicationDirectory();
        while (!std::filesystem::exists(dir / "KSC_DATA"))
        {
            auto parent = dir.parent_path();
            if (parent == dir) break; // reached filesystem root
            dir = parent;
        }
        ChangeDirectory(dir.string().c_str());
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, "KSC");
    SetWindowMinSize(320, 240);
    SetTargetFPS(60);

    RaylibFileOperator       fileParser;
    RaylibGraphicsRenderer renderer;
    GameRunner             game(fileParser, renderer, "locations", "", "C:/KSC_GAME/SAVED_GAMES", true);

    game.loadScene("/BANNERS/START_SCREEN/Start_Screen.json");

    while (!WindowShouldClose())
    {
        // --- Input ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 mouse = GetMousePosition();
            int gx, gy;
            renderer.toGameCoords((int)mouse.x, (int)mouse.y, gx, gy);
            game.registerHit(gx, gy);
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
            game.scroll((int)(-wheel * 30));

        // Enforce 4:3 aspect ratio on resize — snap height to match width.
        if (IsWindowResized())
            SetWindowSize(GetScreenWidth(), GetScreenWidth() * 3 / 4);

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BLACK);
        game.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
