#!/usr/bin/env python3
"""
Rebuild DATA/GAME_STATE/Default_Game_State.json from the LEVELS directory.

For each subdirectory in DATA/LEVELS/, scans the files inside and records
them as the item list for that level.

Usage:
    python SCRIPTS/refresh_game_data.py
"""

import json
from pathlib import Path


def _scalar(v) -> str:
    if isinstance(v, bool):
        return "true" if v else "false"
    if v is None:
        return "null"
    if isinstance(v, str):
        return json.dumps(v)
    return str(v)


def allman_json(obj, indent: int = 0) -> str:
    """Serialize obj to JSON with Allman-style braces (opening brace on its own line)."""
    pad   = "  " * indent
    inner = "  " * (indent + 1)

    if not isinstance(obj, dict):
        return _scalar(obj)

    if not obj:
        return "{}"

    parts = []
    items = list(obj.items())
    for i, (k, v) in enumerate(items):
        comma = "," if i < len(items) - 1 else ""
        if isinstance(v, dict) and v:
            parts.append(f'{inner}"{k}":\n{allman_json(v, indent + 1)}{comma}')
        else:
            parts.append(f'{inner}"{k}": {_scalar(v)}{comma}')

    return f"{pad}{{\n" + "\n".join(parts) + f"\n{pad}}}"

ROOT            = Path(__file__).parent.parent
DATA_ROOT       = ROOT / "DATA"
LEVELS_DIR      = DATA_ROOT / "LEVELS"
GAME_STATE_PATH = DATA_ROOT / "GAME_STATE" / "Default_Game_State.json"


def friendly_name(filename: str) -> str:
    stem = Path(filename).stem
    return stem.replace("_", " ").title()


def level_display_name(dir_name: str) -> str:
    return dir_name.replace("_", " ").title()


def write_level_info(level_dir: Path) -> None:
    assets = {
        f"/LEVELS/{level_dir.name}/{f.name}": friendly_name(f.name)
        for f in sorted(level_dir.iterdir())
        if f.name != "level_info.json" and (f.is_file() or f.is_dir())
    }
    info = {
        "name": level_display_name(level_dir.name),
        "assets": assets,
    }
    out = level_dir / "level_info.json"
    out.write_text(allman_json(info) + "\n", encoding="utf-8")
    print(f"  wrote {out.relative_to(ROOT)}")


def main() -> None:
    levels: dict = {}
    first_level_name = ""
    first_asset_name = ""

    if not LEVELS_DIR.exists():
        print(f"LEVELS directory not found: {LEVELS_DIR}")
    else:
        for level_dir in sorted(LEVELS_DIR.iterdir()):
            if not level_dir.is_dir():
                continue
            assets = {
                f.name: False
                for f in sorted(level_dir.iterdir())
                if f.name != "level_info.json" and (f.is_file() or f.is_dir())
            }
            levels[level_dir.name] = {"isUnlocked": False, "assets": assets}
            print(f"  {level_dir.name}: {list(assets)}")
            write_level_info(level_dir)

            if not first_level_name:
                first_level_name = level_display_name(level_dir.name)
                first_asset_name = friendly_name(next(iter(assets))) if assets else ""

    state = {
        "current_level": first_level_name,
        "current_asset": first_asset_name,
        "levels": levels,
    }

    GAME_STATE_PATH.parent.mkdir(parents=True, exist_ok=True)
    GAME_STATE_PATH.write_text(allman_json(state) + "\n", encoding="utf-8")
    print(f"\nWrote {GAME_STATE_PATH.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
