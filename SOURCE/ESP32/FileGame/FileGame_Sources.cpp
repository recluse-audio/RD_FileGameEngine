/**
 * Unity-build shim: includes all shared and ESP32 .cpp files so the
 * Arduino IDE compiles them as part of the FileGame sketch.
 */

// ESP32 platform
#include "../ESP32GraphicsRenderer.cpp"

// Shared game logic (must match ENGINE_SOURCES in CMAKE/SOURCES.cmake)
#include "../../SHARED/GUI_SECTION/GUISection.cpp"
#include "../../SHARED/GUI_SECTION/SceneListSection.cpp"
#include "../../SHARED/GUI_SECTION/ActiveSceneSection.cpp"
#include "../../SHARED/GUI_SECTION/CharSelectionBox.cpp"
#include "../../SHARED/GUI_SECTION/PasswordEntryComponent.cpp"
#include "../../SHARED/GAME_STATE/GameStateComparison.cpp"
#include "../../SHARED/GAME_RUNNER/GameRunner.cpp"
