/**
 * KSC — ESP32 Arduino entry point
 * Made by Ryan Devens on 2026-02-25
 *
 * Board: ESP32 with integrated TFT display
 * Libraries required:
 *   - TFT_eSPI   (display driver)
 *   - SD         (SD card access)
 *   - SPI        (SPI bus)
 */

#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>

#include "../ESP32FileOperator.h"
#include "../ESP32GraphicsRenderer.h"
#include "../../SHARED/GAME_RUNNER/GameRunner.h"

// --- SD pin config ------------------------------------------------
static const int SD_CS   =  5;
static const int SD_SCK  = 18;
static const int SD_MISO = 19;
static const int SD_MOSI = 23;

SPIClass sdSPI(VSPI);

// --- Touch pin config (XPT2046, software SPI) ---------------------
// Touch is on custom pins that can't share HSPI (TFT) or VSPI (SD).
static const int T_CLK = 25;
static const int T_DIN = 32;
static const int T_DO  = 39;   // input-only GPIO
static const int T_CS  = 33;
static const int T_IRQ = 36;   // input-only GPIO

// Calibration: in landscape (setRotation 1) the XPT2046 axes are transposed —
// raw X channel runs top→bottom (screen Y), raw Y channel runs left→right (screen X).
static const int TOUCH_RAW_X_TOP    = 169;   // raw X when touching top edge
static const int TOUCH_RAW_X_BOTTOM = 1886;  // raw X when touching bottom edge
static const int TOUCH_RAW_Y_LEFT   = 161;   // raw Y when touching left edge
static const int TOUCH_RAW_Y_RIGHT  = 1834;  // raw Y when touching right edge

// --- Globals ------------------------------------------------------
static TFT_eSPI                gTft;
static ESP32FileOperator*      gFileOperator = nullptr;
static ESP32GraphicsRenderer*  gRenderer     = nullptr;
static GameRunner*             gGame         = nullptr;

// --- Touch (XPT2046 software SPI) ---------------------------------

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

// Send 8-bit command, read back 12-bit ADC result.
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

// Returns true and fills screenX/Y when the panel is being touched.
// Prints raw ADC values so you can tune TOUCH_X/Y_MIN/MAX constants.
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

// -----------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(2000); // wait for serial monitor to connect

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
        Serial.println("[KSC] SD init failed — halting.");
        while (true) {}
    }
    Serial.println("[KSC] SD ready.");

    // Build the runner stack and load the opening scene
    gFileOperator = new ESP32FileOperator();
    gRenderer     = new ESP32GraphicsRenderer(gTft);
    gGame         = new GameRunner(*gFileOperator, *gRenderer, "locations", "", "", "/KSC_GAME/SAVED_GAMES");

    gGame->loadScene("/BANNERS/START_SCREEN/Start_Screen.json");

    touchInit();
    Serial.println("[KSC] Touch ready.");
}

static bool gNeedsRedraw  = true;
static bool gPrevTouched  = false;

// -----------------------------------------------------------------
void loop()
{
    // --- Touch input ---
    // Only fires registerHit on the leading edge of a touch (finger-down),
    // then ignores further events until the finger lifts.
    int tx, ty;
    bool touched = touchRead(tx, ty);

    if (touched && !gPrevTouched)
    {
        Serial.printf("[KSC] hit (%d, %d)\n", tx, ty);
        gGame->registerHit(tx, ty);
        gNeedsRedraw = true;
    }
    gPrevTouched = touched;

    // --- Draw (only when scene has changed) ---
    if (gNeedsRedraw)
    {
        gGame->draw();
        gNeedsRedraw = false;
    }
}
