#include "ESP32GraphicsRenderer.h"
#include "ESP32FileOperator.h"
#include <PNGdec.h>
#include <SD.h>

// ---------------------------------------------------------------------------
// PNG decode state — persistent allocations, not stack variables

static TFT_eSPI*  sDrawTft    = nullptr;
static PNG        sPng;
static File       sPngFile;
static uint16_t   sLineBuffer[320];

static void* pngOpen(const char* filename, int32_t* size)
{
    sPngFile = SD.open(filename, FILE_READ);
    if (!sPngFile) { *size = 0; return nullptr; }
    *size = sPngFile.size();
    Serial.printf("[PNG] opened %s  size=%d\n", filename, (int)*size);
    return &sPngFile;
}

static void pngClose(void* handle)
{
    File* f = (File*)handle;
    if (f && *f) f->close();
}

static int32_t pngRead(PNGFILE* /*handle*/, uint8_t* buf, int32_t size)
{
    if (!sPngFile) return 0;
    return sPngFile.read(buf, size);
}

static int32_t pngSeek(PNGFILE* /*handle*/, int32_t pos)
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

// ---------------------------------------------------------------------------

ESP32GraphicsRenderer::ESP32GraphicsRenderer(TFT_eSPI& tft)
: mTft(tft)
{
    sDrawTft = &tft;
}

void ESP32GraphicsRenderer::drawImage(const std::string& path)
{
    std::string full = ESP32FileOperator::sdPath(path);
    Serial.printf("[IMG] drawImage: %s\n", full.c_str());

    mTft.fillScreen(TFT_BLACK);

    int rc = sPng.open(full.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    Serial.printf("[IMG] sPng.open rc=%d\n", rc);

    if (rc == PNG_SUCCESS)
    {
        mTft.startWrite();
        int dec = sPng.decode(nullptr, 0);
        mTft.endWrite();
        sPng.close();
        Serial.printf("[IMG] decode rc=%d\n", dec);
    }
}

void ESP32GraphicsRenderer::drawText(const std::string& path, int x, int y)
{
    File file = SD.open(ESP32FileOperator::sdPath(path).c_str(), FILE_READ);
    if (!file) return;

    mTft.setTextWrap(true);
    int    curY = y;
    String line;

    while (file.available())
    {
        char c = file.read();
        if (c != '\n') { line += c; continue; }

        int hashes = 0;
        while (hashes < (int)line.length() && line[hashes] == '#') hashes++;
        String display = (hashes > 0) ? line.substring(hashes + 1) : line;

        uint8_t sz    = (hashes == 1) ? 3 : (hashes == 2) ? 2 : 1;
        int     lineH = sz * 8 + 4;
        mTft.setTextSize(sz);
        mTft.setTextColor(hashes > 0 ? TFT_WHITE : TFT_LIGHTGREY, TFT_BLACK);
        mTft.drawString(display, x, curY);
        curY += lineH;
        line = "";
    }
    file.close();
}

void ESP32GraphicsRenderer::drawSVG(const std::string& /*path*/,
                                    int /*x*/, int /*y*/,
                                    int /*w*/, int /*h*/) {}

void ESP32GraphicsRenderer::drawButton(const std::string& label, int x, int y, int w, int h)
{
    mTft.fillRect(x, y, w, h, TFT_BLACK);
    mTft.drawRect(x, y, w, h, TFT_WHITE);
    mTft.setTextSize(1);
    mTft.setTextColor(TFT_WHITE, TFT_BLACK);
    int textX = x + (w - (int)label.size() * 6) / 2;
    int textY = y + (h - 8) / 2;
    mTft.drawString(label.c_str(), textX, textY);
}
