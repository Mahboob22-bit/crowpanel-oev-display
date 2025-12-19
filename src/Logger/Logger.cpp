#include "Logger.h"

void Logger::init(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void Logger::info(const char* tag, const char* message) {
    Serial.printf("[%s] %s\n", tag, message);
}

void Logger::error(const char* tag, const char* message) {
    Serial.printf("[%s] ERROR: %s\n", tag, message);
}
