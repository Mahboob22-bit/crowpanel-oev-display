#include "SystemMonitor.h"
#include "../Logger/Logger.h"

void SystemMonitor::begin() {
    xTaskCreatePinnedToCore(
        taskCode,
        "SystemTask",
        4096,
        NULL,
        1,
        &taskHandle,
        1
    );
}

void SystemMonitor::taskCode(void* pvParameters) {
    for(;;) {
        Logger::printf("SYSTEM", "Core %d | Heap: %d KB | Stack: %d bytes",
                     xPortGetCoreID(),
                     ESP.getFreeHeap() / 1024,
                     uxTaskGetStackHighWaterMark(NULL));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
