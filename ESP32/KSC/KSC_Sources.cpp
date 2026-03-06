/**
 * Unity-build shim: includes all shared and ESP32 .cpp files so the
 * Arduino IDE compiles them as part of the KSC sketch.
 */

// ESP32 platform
#include "../ESP32GraphicsRenderer.cpp"

// Shared game logic
#include "../../SHARED/ZONE/Zone.cpp"
#include "../../SHARED/SCENE/Scene.cpp"
#include "../../SHARED/SCENE/SceneFactory.cpp"
#include "../../SHARED/SCENE_VIEW/SceneView.cpp"
#include "../../SHARED/CONTROLS_VIEW/ControlsView.cpp"
#include "../../SHARED/GAME_STATE/GameStateComparison.cpp"
#include "../../SHARED/GAME_RUNNER/GameStartManager.cpp"
#include "../../SHARED/GAME_RUNNER/GameRunner.cpp"
