# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A "file game" engine — a puzzle/riddle game where the player navigates between file assets (text, images, SVGs) across levels. Targets two platforms:
- **Raylib desktop** (Windows, 320×240 logical resolution scaled 2×)
- **ESP32** with ILI9341 TFT display via Arduino/TFT_eSPI

## Build & Test Commands

All helper scripts live in `SCRIPTS/` and are the primary way to build/run.

### Tests (Catch2, no platform deps)
```bash
python SCRIPTS/build_tests.py
# Produces: BUILD/Debug/Tests.exe
BUILD/Debug/Tests.exe                        # run all tests
BUILD/Debug/Tests.exe "[GameRunner]"         # run one tag
```

### Raylib Desktop Build
```bash
python SCRIPTS/build_raylib.py
# Produces: BUILD/Release/GameEngine_Raylib.exe
```

### Run the Game
```bash
# First sync build+data to C:\RD_GAME:
python SCRIPTS/sync_with_desktop.py
python SCRIPTS/sync_with_desktop.py --dry-run   # preview

# Then launch:
python SCRIPTS/run_game.py
```

### Data Maintenance
```bash
# After adding/removing level files, regenerate level_info.json + Default_Game_State.json:
python SCRIPTS/refresh_game_data.py

# Sync DATA/ to ESP32 SD card (volume label must be KSC_SD):
python SCRIPTS/sync_to_sd.py
python SCRIPTS/sync_to_sd.py --dry-run
```

## Architecture

### Platform abstraction

Two abstract interfaces decouple the engine from the platform:

- **`FileOperator`** (`SOURCE/SHARED/FILE_OPERATOR/FileOperator.h`) — load/write/append files and list directories. Platform implementations: `RaylibFileOperator` (desktop), `ESP32FileOperator`.
- **`GraphicsRenderer`** (`SOURCE/SHARED/GRAPHICS_RENDERER/GraphicsRenderer.h`) — draw images, text, SVG, buttons, rects, labels. Platform implementations: `RaylibGraphicsRenderer`, `ESP32GraphicsRenderer`.

### Engine core (`SOURCE/SHARED/`)

| Class | Role |
|---|---|
| `GameRunner` | Top-level coordinator. Owns `GUISection` list, `AssetListSection`, and `Level` list. Loads layout from `/GUI/gui_layout.json` and level metadata from `/LEVELS/<id>/level_info.json`. Drives the frame loop via `draw()` / `registerHit()`. |
| `GUISection` | Named rectangular region of the 320×240 window. Defined in `gui_layout.json`. |
| `AssetListSection` | `GUISection` subclass that renders a scrollable list of level asset names. Updated when the active level changes. |
| `Level` | Plain struct: id (dir name), display name, `map<path, friendlyName>` assets. |
| `GameStateComparison` | Compares two `GameState` snapshots. |

### Data layout (`DATA/`)

```
DATA/
  GUI/
    gui_layout.json          # Section layout (ids, x/y/w/h)
    OcrB2.ttf
  LEVELS/
    LEVEL_1/
      level_info.json        # {"name": ..., "assets": {path: friendlyName}}
      asset_1.md
      asset_2.png
    LEVEL_2/
      ...
  GAME_STATE/
    Default_Game_State.json  # Persisted state: current level/asset + unlock flags
```

`refresh_game_data.py` regenerates `level_info.json` for every level dir and rewrites `Default_Game_State.json` from the actual files present. Run it whenever level content changes.

### Tests (`TESTS/`)

Tests use Catch2 v3 (fetched by CMake). Test helpers live in `TESTS/UTIL/`:
- `TestFileOperator` — in-memory `FileOperator` backed by `std::map<string,string> files`
- `NullGraphicsRenderer` — no-op `GraphicsRenderer`

Active test sources are listed in `CMAKE/TESTS.cmake`. To add a new test file, add it there.

### ESP32 target (`SOURCE/ESP32/`)

Arduino sketches in `SOURCE/ESP32/KSC/KSC.ino`. TFT_eSPI configuration for ILI9341 + ESP32 HSPI is in `SOURCE/ESP32/CONFIG/`. Include it in sketches with:
```cpp
#define USER_SETUP_LOADED
#include "../CONFIG/User_Setup_TFT_eSPI.h"
#include <TFT_eSPI.h>
```

### CMake structure

- `CMakeLists.txt` — root; defines `Tests` target (always built) and optional `GameEngine_Raylib` target (`-DBUILD_RAYLIB=ON`)
- `CMAKE/SOURCES.cmake` — `ENGINE_SOURCES` list (shared C++ files compiled into both targets)
- `CMAKE/TESTS.cmake` — `TEST_SOURCES` list
- Build output goes to `BUILD/`; dependencies (Catch2, raylib) are fetched via `FetchContent`
