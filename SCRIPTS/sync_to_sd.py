#!/usr/bin/env python3
"""
Sync game files from repo to SD card (KSC_SD volume).

The repo is the source of truth. This script copies the DATA folder structure
to the SD card, preserving the /DATA/ path.

Usage:
    python SCRIPTS/sync_to_sd.py
    python SCRIPTS/sync_to_sd.py --dry-run  # preview without copying
"""

import argparse
import shutil
import string
import os
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent
SOURCE    = REPO_ROOT / "DATA"

def find_sd_card():
    """Find the drive letter for volume named KSC_SD."""
    if os.name == 'nt':  # Windows
        try:
            import win32api
            drives = win32api.GetLogicalDriveStrings()
            drives = drives.split('\000')[:-1]
            for drive in drives:
                try:
                    volume_name = win32api.GetVolumeInformation(drive)[0]
                    if volume_name == "KSC_SD":
                        return Path(drive)
                except:
                    continue
        except ImportError:
            # Fallback: use `vol` command to read volume label without pywin32
            print("Note: pywin32 not installed, reading volume labels via vol command...")
            for letter in string.ascii_uppercase:
                drive = f"{letter}:\\"
                if not os.path.exists(drive):
                    continue
                try:
                    result = subprocess.run(
                        ["vol", f"{letter}:"],
                        capture_output=True, text=True, shell=True
                    )
                    first_line = result.stdout.splitlines()[0] if result.stdout else ""
                    if "KSC_SD" in first_line:
                        print(f"  Found KSC_SD at {drive}")
                        return Path(drive)
                except Exception:
                    continue
    return None

DEST = None  # Will be set in main() after finding SD card

# What to exclude (by extension or name)
EXCLUDE_EXTENSIONS = {".ino", ".cpp", ".h"}
EXCLUDE_NAMES      = {".git", ".gitignore", "__pycache__"}


def should_copy(src: Path, dest: Path) -> bool:
    """Check if src should be copied to dest (based on mtime/size)."""
    if not dest.exists():
        return True
    src_stat = src.stat()
    dest_stat = dest.stat()
    # Copy if size differs or source is newer
    return (src_stat.st_size != dest_stat.st_size or
            src_stat.st_mtime > dest_stat.st_mtime)


def is_excluded(path: Path) -> bool:
    """Check if path should be excluded from sync."""
    if path.suffix in EXCLUDE_EXTENSIONS:
        return True
    if path.name in EXCLUDE_NAMES:
        return True
    return False


def sync_folder(src_folder: Path, dest_folder: Path, dry_run: bool):
    """Recursively sync src_folder to dest_folder."""
    if not src_folder.exists():
        print(f"  Skipping {src_folder.name}/ (not found in repo)")
        return 0, 0

    copied = skipped = 0

    for src_file in src_folder.rglob("*"):
        if src_file.is_dir():
            continue
        if is_excluded(src_file):
            continue

        rel_path = src_file.relative_to(src_folder)
        dest_file = dest_folder / rel_path

        if should_copy(src_file, dest_file):
            if dry_run:
                print(f"  [DRY] {rel_path}")
            else:
                dest_file.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src_file, dest_file)
                print(f"  {rel_path}")
            copied += 1
        else:
            skipped += 1

    return copied, skipped


def main():
    global DEST

    parser = argparse.ArgumentParser(
        description="Sync game files to SD card (KSC_SD volume).",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument("--dry-run", action="store_true",
                        help="Preview changes without copying")

    args = parser.parse_args()

    if not SOURCE.exists():
        print(f"Error: Source folder not found: {SOURCE}")
        return

    # Find SD card with volume name KSC_SD
    sd_drive = find_sd_card()
    if sd_drive is None:
        print("Error: Could not find SD card with volume name 'KSC_SD'")
        print("Please ensure SD card is inserted and named 'KSC_SD'")
        return

    # Mirror the DATA folder structure on the SD card
    DEST = sd_drive / "DATA"

    print(f"Source: {SOURCE}")
    print(f"Dest:   {DEST}")
    if args.dry_run:
        print("[DRY RUN MODE - no files will be copied]\n")
    else:
        print()

    total_copied = total_skipped = 0

    # First, copy root-level files
    print("Root files:")
    for src_file in SOURCE.iterdir():
        if src_file.is_file() and not is_excluded(src_file):
            dest_file = DEST / src_file.name

            if should_copy(src_file, dest_file):
                if args.dry_run:
                    print(f"  [DRY] {src_file.name}")
                else:
                    DEST.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(src_file, dest_file)
                    print(f"  {src_file.name}")
                total_copied += 1
            else:
                total_skipped += 1

    # Then copy subdirectories
    subdirs = [d for d in SOURCE.iterdir() if d.is_dir()]
    if not subdirs:
        print(f"No subdirectories found in {SOURCE}")
        return

    for src_folder in sorted(subdirs):
        folder_name = src_folder.name
        dest_folder = DEST / folder_name

        print(f"\n{folder_name}/")
        copied, skipped = sync_folder(src_folder, dest_folder, args.dry_run)
        total_copied += copied
        total_skipped += skipped

    print(f"\n{'='*50}")
    print(f"  Copied  : {total_copied}")
    print(f"  Skipped : {total_skipped} (up-to-date)")
    print(f"{'='*50}")

    if args.dry_run:
        print("\nRun without --dry-run to actually copy files.")


if __name__ == "__main__":
    main()
