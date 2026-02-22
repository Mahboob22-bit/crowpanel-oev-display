#ifndef DEVICE_IDENTITY_H
#define DEVICE_IDENTITY_H

#include <Arduino.h>

class DeviceIdentity {
public:
    DeviceIdentity();

    void begin();

    const char* getDeviceId();
    const char* getFirmwareVersion();
    uint32_t getFlashSizeMB();
    uint32_t getPsramSizeKB();

private:
    char _deviceId[16]; // "CP-" + 12 hex chars + null
};

#endif // DEVICE_IDENTITY_H
