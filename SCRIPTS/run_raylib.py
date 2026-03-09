#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).parent.parent
EXE  = ROOT / "BUILD" / "Release" / "GameEngine_Raylib.exe"


def main():
    if not EXE.exists():
        print(f"Error: executable not found: {EXE}")
        print("Run 'python SCRIPTS/build_raylib.py' first.")
        sys.exit(1)

    try:
        subprocess.run([str(EXE)], check=True)
    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)


if __name__ == "__main__":
    main()
