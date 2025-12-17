// Stub header for Arduino.h - nur für clangd IntelliSense
// Die echten Definitionen kommen aus dem ESP32 Arduino Framework im Docker Container

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// FreeRTOS - basic types
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;
typedef long BaseType_t;

// FreeRTOS functions
extern "C" {
  BaseType_t xTaskCreatePinnedToCore(
    void (*)(void*),
    const char*,
    uint32_t,
    void*,
    uint32_t,
    TaskHandle_t*,
    BaseType_t
  );

  void vTaskDelay(TickType_t);
  int xPortGetCoreID();
}

// FreeRTOS macros
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

// Arduino constants
#define LED_BUILTIN 2
#define HIGH 1
#define LOW 0
#define INPUT 0x01
#define OUTPUT 0x02
#define INPUT_PULLUP 0x05

// Arduino types
typedef bool boolean;
typedef uint8_t byte;

// Arduino functions
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
void delay(uint32_t ms);
uint32_t millis();
uint32_t micros();

// String class (minimal)
class String {
public:
  String();
  String(const char* str);
  String(int num);
  String(unsigned int num);
  String(long num);
  String(unsigned long num);
  String operator+(const String& rhs);
  const char* c_str() const;
};

// Serial class
class HardwareSerial {
public:
  void begin(unsigned long baud);
  void end();
  int available();
  int read();
  size_t write(uint8_t);
  size_t write(const uint8_t* buffer, size_t size);
  void print(const char*);
  void print(const String&);
  void println(const char*);
  void println(const String&);
  void println();
};

extern HardwareSerial Serial;

// Setup and loop
void setup();
void loop();
