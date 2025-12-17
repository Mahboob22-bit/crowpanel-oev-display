// Stub header for Arduino.h - nur für clangd IntelliSense
// Die echten Definitionen kommen aus dem ESP32 Arduino Framework im Docker Container

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// FreeRTOS - basic types
typedef void* TaskHandle_t;
typedef void* QueueHandle_t;
typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

// FreeRTOS constants
#define pdTRUE ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)
#define pdPASS (pdTRUE)
#define pdFAIL (pdFALSE)
#define portMAX_DELAY ((TickType_t)0xffffffffUL)
// NULL already defined in stddef.h

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
  void vTaskDelete(TaskHandle_t);
  int xPortGetCoreID();
  UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t);

  QueueHandle_t xQueueCreate(UBaseType_t, UBaseType_t);
  BaseType_t xQueueSend(QueueHandle_t, const void*, TickType_t);
  BaseType_t xQueueReceive(QueueHandle_t, void*, TickType_t);
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
  void print(int);
  void print(unsigned int);
  void print(long);
  void print(unsigned long);
  void println(const char*);
  void println(const String&);
  void println(int);
  void println(unsigned int);
  void println(long);
  void println(unsigned long);
  void println();
  void printf(const char* format, ...);
};

extern HardwareSerial Serial;

// ESP class
class EspClass {
public:
  uint32_t getFreeHeap();
  uint32_t getChipCores();
  uint32_t getCpuFreqMHz();
  uint32_t getFlashChipSize();
  uint32_t getPsramSize();
};

extern EspClass ESP;

// Interrupt functions
typedef void (*voidFuncPtr)(void);
void attachInterrupt(uint8_t pin, voidFuncPtr handler, int mode);
void detachInterrupt(uint8_t pin);
int digitalPinToInterrupt(uint8_t pin);

// Interrupt modes
#define FALLING 0x02
#define RISING 0x03
#define CHANGE 0x04

// ISR attribute
#define IRAM_ATTR __attribute__((section(".iram1")))

// Setup and loop
void setup();
void loop();
