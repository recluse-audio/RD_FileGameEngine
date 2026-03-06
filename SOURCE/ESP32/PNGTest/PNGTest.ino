/**
 * PNGTest — minimal sketch to verify PNG display on TFT.
 * Displays /KSC_DATA/BANNERS/START_SCREEN/Start_Screen_320x240.png
 * and nothing else.
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <PNGdec.h>

// --- SD on VSPI ---
#define SD_CS    5
#define SD_SCK  18
#define SD_MISO 19
#define SD_MOSI 23
SPIClass sdSPI(VSPI);

// --- Target file ---
const char* PNG_PATH = "/KSC_DATA/BANNERS/START_SCREEN/Start_Screen_320x240.png";

// --- Globals ---
TFT_eSPI tft;

static TFT_eSPI*  sDrawTft   = nullptr;
static PNG        sPng;
static File       sPngFile;
static uint16_t   sLineBuffer[320];

// --- PNG callbacks (same pattern as reference PNGViewer.h) ---

static void* pngOpen(const char* filename, int32_t* size)
{
    sPngFile = SD.open(filename, FILE_READ);
    if (!sPngFile) { *size = 0; Serial.printf("[PNG] OPEN FAILED: %s\n", filename); return nullptr; }
    *size = sPngFile.size();
    Serial.printf("[PNG] opened %s  size=%d\n", filename, (int)*size);
    return &sPngFile;
}

static void pngClose(void* handle)
{
    File* f = (File*)handle;
    if (f && *f) f->close();
}

static int32_t pngRead(PNGFILE*, uint8_t* buf, int32_t size)
{
    if (!sPngFile) return 0;
    return sPngFile.read(buf, size);
}

static int32_t pngSeek(PNGFILE*, int32_t pos)
{
    if (!sPngFile) return 0;
    return sPngFile.seek(pos) ? pos : 0;
}

static int pngDraw(PNGDRAW* pDraw)
{
    if (!sDrawTft || (int)pDraw->iWidth > 320) return 0;
    sPng.getLineAsRGB565(pDraw, sLineBuffer, PNG_RGB565_BIG_ENDIAN, 0xFFFFFFFF);
    sDrawTft->pushImage(0, (int)pDraw->y, (int)pDraw->iWidth, 1, sLineBuffer);
    return 1;
}

// --- Setup ---

void setup()
{
    Serial.begin(115200);
    delay(3000);
    Serial.println("[TEST] booting");

    // Display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Initialising...", 10, 10, 2);
    Serial.println("[TEST] TFT init done");

    // SD — explicit VSPI init
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
        tft.fillScreen(TFT_RED);
        tft.drawString("SD FAILED", 10, 10, 4);
        Serial.println("[TEST] SD init FAILED");
        while (true) {}
    }
    Serial.println("[TEST] SD init OK");

    // Display PNG
    sDrawTft = &tft;
    tft.fillScreen(TFT_BLACK);

    int rc = sPng.open(PNG_PATH, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    Serial.printf("[TEST] sPng.open rc=%d\n", rc);

    if (rc == PNG_SUCCESS)
    {
        tft.startWrite();
        int dec = sPng.decode(nullptr, 0);
        tft.endWrite();
        sPng.close();
        Serial.printf("[TEST] decode rc=%d  size=%dx%d\n", dec, sPng.getWidth(), sPng.getHeight());
    }
    else
    {
        tft.fillScreen(TFT_RED);
        tft.drawString("PNG open failed", 10, 10, 2);
        Serial.printf("[TEST] PNG open failed rc=%d\n", rc);
    }
}

void loop() {}
