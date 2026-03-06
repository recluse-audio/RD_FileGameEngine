/**
 * Made by Ryan Devens on 2026-02-27
 */

#pragma once
#include <string>

class FileOperator;

/**
 * Handles save-slot creation when the player chooses to save from the start screen.
 * Copies the entire GAME_STATE directory into SAVE_DIR/KSC_SLOT_N/, where N is the
 * next available slot index (determined by scanning SAVE_DIR for existing KSC_SLOT_*
 * subdirectories).
 *
 * Constructed with a FileOperator reference and the platform-specific save directory
 * path. When saveDir is empty, save() is a no-op.
 */
class GameStartManager
{
public:
    GameStartManager(FileOperator& fileOperator, std::string saveDir);

    /**
     * Override the save directory at runtime. Replaces the directory set at
     * construction. Pass an empty string to disable saving.
     */
    void        setSaveDir(const std::string& dir);
    std::string getSaveDir() const;

    /**
     * Copy KSC_DATA/GAME_STATE/ into saveDir/KSC_SLOT_N/ where N is the next
     * available slot index. No-op if saveDir is empty.
     */
    void save();

private:
    FileOperator& mFileOperator;
    std::string   mSaveDir;

    int findNextSlotIndex();
};
