#include <Arduino.h>

void setup() {
    // Hardware UART Serial initialisieren (UART0)
    Serial.begin(115200);
    delay(1000); // Warte auf Serial-Stabilität

    Serial.println("\n\n=================================");
    Serial.println("CrowPanel ÖV Display - Test");
    Serial.println("=================================");
    Serial.println("ESP32-S3 gestartet!");
    Serial.println("Core: " + String(xPortGetCoreID()));
    Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
    Serial.println("=================================\n");
}

void loop() {
    static uint32_t counter = 0;

    Serial.print("Loop #");
    Serial.print(counter++);
    Serial.print(" - Core: ");
    Serial.print(xPortGetCoreID());
    Serial.print(" - Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    delay(2000);
}
