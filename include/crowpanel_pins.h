#ifndef CROWPANEL_PINS_H
#define CROWPANEL_PINS_H

// E-Paper Display Pins (CrowPanel 4.2")
#define EPD_PWR_PIN     7   // Power Enable
#define EPD_BUSY_PIN    48  // Busy Signal
#define EPD_RST_PIN     47  // Reset
#define EPD_DC_PIN      46  // Data/Command
#define EPD_CS_PIN      45  // Chip Select
#define EPD_SCK_PIN     39  // SPI Clock
#define EPD_MOSI_PIN    40  // SPI MOSI

// Buttons
#define BTN_MENU        2   // Menu Button
#define BTN_EXIT        1   // Exit Button
#define BTN_ROTARY_A    3   // Rotary Encoder A
#define BTN_ROTARY_B    4   // Rotary Encoder B
#define BTN_ROTARY_SW   5   // Rotary Switch

// SD Card
#define SD_CS           6
#define SD_MOSI         11
#define SD_MISO         13
#define SD_SCK          12

#endif // CROWPANEL_PINS_H
