/**
 * Opens a native OS file-picker dialog rooted at the save directory,
 * filtered to JSON save files for the active game.
 * Returns the selected file path, or "" if cancelled.
 *
 * Implemented in SaveFileBrowser.cpp to isolate Windows headers from
 * raylib.h (they define conflicting symbols: CloseWindow, DrawText, Rectangle…)
 */
#pragma once
#include <string>

std::string browseForSaveFile(const std::string& gameId, const std::string& saveDir);
