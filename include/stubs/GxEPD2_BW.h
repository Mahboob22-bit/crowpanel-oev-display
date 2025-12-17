// Stub header for GxEPD2 - nur für clangd IntelliSense
#pragma once

#include <stdint.h>

// Forward declarations
class GxEPD2_EPD;

// Dummy display class for syntax checking
class GxEPD2_420_GYE042A87 {
public:
    static const int WIDTH = 400;
    static const int HEIGHT = 300;

    GxEPD2_420_GYE042A87(int8_t cs, int8_t dc, int8_t rst, int8_t busy) {}
};

// Template class for display
template<class T, int page_height>
class GxEPD2_BW {
public:
    GxEPD2_BW(T display) {}

    void init(uint32_t serial_diag_bitrate = 0, bool initial = true, uint16_t reset_duration = 2, bool pulldown_rst_mode = false);
    void setRotation(uint8_t r);
    void setFont(const void* f);
    void setTextColor(uint16_t c);
    void setFullWindow();
    bool nextPage();
    void firstPage();
    void fillScreen(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    size_t print(const char* str);
    size_t print(int num);
    size_t println(const char* str);
    size_t println(int num);
    size_t println();

    void hibernate();
};

// Colors
#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF
