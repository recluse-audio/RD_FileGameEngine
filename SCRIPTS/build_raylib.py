#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).parent.parent
BUILD = ROOT / "BUILD"


def run(cmd, cwd):
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def main():
    BUILD.mkdir(exist_ok=True)

    try:
        run(["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DBUILD_RAYLIB=ON", str(ROOT)], BUILD)
        run(["cmake", "--build", ".", "--target", "GameEngine_Raylib", "--config", "Release"], BUILD)
    except subprocess.CalledProcessError:
        sys.exit(1)


if __name__ == "__main__":
    main()
