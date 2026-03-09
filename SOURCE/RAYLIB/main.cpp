/**
 * RD File Game Engine — Raylib desktop entry point
 * Made by Ryan Devens on 2026-02-25
 */

#include "raylib.h"
#include "RaylibFileOperator.h"
#include "RaylibGameLibrary.h"
#include "GamePickerScreen.h"
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

    RaylibGraphicsRenderer renderer;

    while (!WindowShouldClose())
    {
        // Discover available FILE_GAMEs each time we return to the library.
        std::vector<GameEntry> games = RaylibGameLibrary::discover();

        // Show the picker and wait for a selection.
        GamePickerScreen picker(games);
        std::string dataPath = picker.run();

        if (dataPath.empty() || WindowShouldClose())
            break;

        // Run the selected game until the user clicks "home" or closes the window.
        renderer.setDataRoot(dataPath);
        RaylibFileOperator fileOperator(dataPath);
        GameRunner         game(fileOperator, renderer);

        while (!WindowShouldClose() && !game.wantsToExitToLibrary())
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Vector2 mouse = GetMousePosition();
                int gx, gy;
                renderer.toGameCoords((int)mouse.x, (int)mouse.y, gx, gy);
                game.registerHit(gx, gy);
            }

            if (IsWindowResized())
                SetWindowSize(GetScreenWidth(), GetScreenWidth() * 3 / 4);

            BeginDrawing();
            ClearBackground(BLACK);
            game.draw();
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
