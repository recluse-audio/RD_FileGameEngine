/**
 * Opens a native OS folder-picker dialog and returns the selected game's
 * DATA directory path, or "" if cancelled.
 *
 * Implemented in GameFolderBrowser.cpp to isolate Windows headers from
 * raylib.h (they define conflicting symbols: CloseWindow, DrawText, Rectangle…)
 */
#pragma once
#include <string>

std::string browseForGameFolder();
