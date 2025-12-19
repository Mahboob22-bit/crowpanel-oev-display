#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger {
public:
    static void init(unsigned long baudRate);
    static void info(const char* tag, const char* message);
    static void error(const char* tag, const char* message);
    
    // Hilfsfunktion für formatierte Ausgabe
    template<typename... Args>
    static void printf(const char* tag, const char* format, Args... args) {
        Serial.printf("[%s] ", tag);
        Serial.printf(format, args...);
        Serial.println();
    }
};

#endif // LOGGER_H
