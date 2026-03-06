/**
 * RD File Game Engine — Raylib desktop entry point
 * Made by Ryan Devens on 2026-02-25
 */

#include "raylib.h"
#include "RaylibFileOperator.h"
#include "RaylibGraphicsRenderer.h"
#include "../SHARED/GAME_RUNNER/GameRunner.h"

static const int SCALE    = 2;
static const int SCREEN_W = 320 * SCALE;
static const int SCREEN_H = 240 * SCALE;

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, "RD File Game Engine");
    SetWindowMinSize(320, 240);
    SetTargetFPS(60);

    RaylibFileOperator     fileParser;
    RaylibGraphicsRenderer renderer;
    GameRunner             game(fileParser, renderer);

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 mouse = GetMousePosition();
            int gx, gy;
            renderer.toGameCoords((int)mouse.x, (int)mouse.y, gx, gy);
            game.registerHit(gx, gy);
        }

        // Enforce 4:3 aspect ratio on resize — snap height to match width.
        if (IsWindowResized())
            SetWindowSize(GetScreenWidth(), GetScreenWidth() * 3 / 4);

        BeginDrawing();
        ClearBackground(BLACK);
        game.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
