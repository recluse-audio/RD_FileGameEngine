@echo off
REM Sync game files from repo to SD card (KSC_SD volume)
REM Uses robocopy for efficient mirroring (only copies changed files)

set SOURCE=C:\REPOS\PROJECTS\RD_FILE_GAME_ENGINE\DATA
set DEST=D:\DATA

echo Syncing DATA/ files to SD card...
echo Source: %SOURCE%
echo Dest:   %DEST%
echo.

REM Mirror everything in DATA/ to D:/DATA/
robocopy "%SOURCE%" "%DEST%" /MIR /XF *.ino *.cpp *.h .gitignore /XD .git /R:1 /W:1 /NP

echo.
echo ================================================
echo Sync complete!
echo ================================================
echo.
echo Next step: Copy D:\DATA\ to your SD card root
pause
