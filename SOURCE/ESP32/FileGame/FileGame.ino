/**
 * FileGame — ESP32 Arduino entry point for RD_FileGameSystem
 * Made by Ryan Devens on 2026-03-10
 *
 * Board: ESP32 with ILI9341 2.8" TFT display (E32R28T)
 * Libraries required:
 *   - TFT_eSPI   (display driver, configured via CONFIG/User_Setup_TFT_eSPI.h)
 *   - SD         (SD card access)
 *   - SPI        (SPI bus)
 *   - PNGdec     (PNG image decoding)
 *
 * SD card layout (mirrors desktop C:\FILE_GAMES\GAMES\<game>\):
 *   /FILE_GAMES/GAMES/BBB/GUI/gui_layout.json
 *   /FILE_GAMES/GAMES/BBB/LEVELS/...
 *   /FILE_GAMES/GAMES/BBB/GAME_STATE/Default_Game_State.json
 *   /FILE_GAME_SAVES/BBB_save.json  <- written via writeAbsolute, not under DATA_ROOT
 *
 * To switch games: update GAME_NAME below and re-flash.
 */

#define USER_SETUP_LOADED
#include "../CONFIG/User_Setup_TFT_eSPI.h"
#include <TFT_eSPI.h>

#include <SPI.h>
#include <SD.h>

#include "../ESP32FileOperator.h"
#include "../ESP32GraphicsRenderer.h"
#include "../../SHARED/GAME_RUNNER/GameRunner.h"

// --- Game selection — update this string and re-flash to switch games ----
#define GAME_NAME "BBB"

// --- Data root on the SD card (mirrors C:\FILE_GAMES\GAMES\<game>\) ------
static const char* DATA_ROOT = "/FILE_GAMES/GAMES/" GAME_NAME;

// --- SD pin config (VSPI — separate from TFT on HSPI) -------------------
static const int SD_CS   =  5;
static const int SD_SCK  = 18;
static const int SD_MISO = 19;
static const int SD_MOSI = 23;

SPIClass sdSPI(VSPI);

// --- Touch pin config (XPT2046, software SPI) ----------------------------
static const int T_CLK = 25;
static const int T_DIN = 32;
static const int T_DO  = 39;   // input-only GPIO
static const int T_CS  = 33;
static const int T_IRQ = 36;   // input-only GPIO

// Calibration: in landscape (setRotation 1) the XPT2046 axes are transposed —
// raw X channel runs top→bottom (screen Y), raw Y channel runs left→right (screen X).
static const int TOUCH_RAW_X_TOP    = 169;
static const int TOUCH_RAW_X_BOTTOM = 1886;
static const int TOUCH_RAW_Y_LEFT   = 161;
static const int TOUCH_RAW_Y_RIGHT  = 1834;

// --- Globals -------------------------------------------------------------
static TFT_eSPI                gTft;
static ESP32FileOperator       gFileOperator;
static ESP32GraphicsRenderer*  gRenderer = nullptr;
static GameRunner*             gGame     = nullptr;

// --- Touch (XPT2046 software SPI) ----------------------------------------

static void touchInit()
{
    pinMode(T_CLK, OUTPUT);
    pinMode(T_DIN, OUTPUT);
    pinMode(T_DO,  INPUT);
    pinMode(T_CS,  OUTPUT);
    pinMode(T_IRQ, INPUT);
    digitalWrite(T_CS,  HIGH);
    digitalWrite(T_CLK, LOW);
}

static uint16_t xpt2046Read(uint8_t cmd)
{
    uint16_t val = 0;
    for (int i = 7; i >= 0; i--)
    {
        digitalWrite(T_DIN, (cmd >> i) & 1);
        digitalWrite(T_CLK, HIGH); delayMicroseconds(1);
        digitalWrite(T_CLK, LOW);  delayMicroseconds(1);
    }
    for (int i = 11; i >= 0; i--)
    {
        digitalWrite(T_CLK, HIGH); delayMicroseconds(1);
        val |= (uint16_t)digitalRead(T_DO) << i;
        digitalWrite(T_CLK, LOW);  delayMicroseconds(1);
    }
    return val;
}

static bool touchRead(int& screenX, int& screenY)
{
    if (digitalRead(T_IRQ) == HIGH) return false;

    digitalWrite(T_CS, LOW);
    delayMicroseconds(10);
    uint16_t rawX = xpt2046Read(0xD0); // X channel
    uint16_t rawY = xpt2046Read(0x90); // Y channel
    digitalWrite(T_CS, HIGH);

    if (digitalRead(T_IRQ) == HIGH) return false; // lifted during read

    Serial.printf("[TOUCH] raw x=%d y=%d\n", rawX, rawY);

    // Axes are transposed in landscape: raw Y → screen X, raw X → screen Y
    screenX = constrain(map(rawY, TOUCH_RAW_Y_LEFT,   TOUCH_RAW_Y_RIGHT,  0, 319), 0, 319);
    screenY = constrain(map(rawX, TOUCH_RAW_X_TOP,    TOUCH_RAW_X_BOTTOM, 0, 239), 0, 239);
    return true;
}

// -------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(2000);

    // Display
    gTft.init();
    gTft.setRotation(1);
    gTft.fillScreen(TFT_BLACK);

    // SD card — explicit VSPI so it doesn't clash with TFT on HSPI
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    bool sdOk = false;
    int freqs[] = { 25000000, 10000000, 4000000 };
    for (int f : freqs)
    {
        if (SD.begin(SD_CS, sdSPI, f)) { sdOk = true; break; }
        delay(100);
    }
    if (!sdOk)
    {
        gTft.drawString("SD init failed", 10, 10);
        Serial.println("[FileGame] SD init failed — halting.");
        while (true) {}
    }
    Serial.println("[FileGame] SD ready.");

    // Configure data root on both file operator and renderer
    gFileOperator.setDataRoot(DATA_ROOT);
    gRenderer = new ESP32GraphicsRenderer(gTft);
    gRenderer->setDataRoot(DATA_ROOT);

    gGame = new GameRunner(gFileOperator, *gRenderer);
    gGame->setSaveDir("/FILE_GAME_SAVES"); // absolute path on SD card, not under DATA_ROOT

    touchInit();
    Serial.println("[FileGame] Touch ready.");
}

static bool gNeedsRedraw = true;
static bool gPrevTouched = false;

// -------------------------------------------------------------------------
void loop()
{
    // Touch input — fires registerHit only on the leading edge (finger-down).
    int tx, ty;
    bool touched = touchRead(tx, ty);

    if (touched && !gPrevTouched)
    {
        Serial.printf("[FileGame] hit (%d, %d)\n", tx, ty);
        gGame->registerHit(tx, ty);
        gNeedsRedraw = true;
    }
    gPrevTouched = touched;

    // Draw only when needed
    if (gNeedsRedraw)
    {
        gGame->draw();
        gNeedsRedraw = false;
    }
}
