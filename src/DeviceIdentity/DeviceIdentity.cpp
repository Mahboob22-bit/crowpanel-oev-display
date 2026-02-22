#include "DeviceIdentity.h"
#include "../Logger/Logger.h"
#include "version.h"

DeviceIdentity::DeviceIdentity() {
    memset(_deviceId, 0, sizeof(_deviceId));
}

void DeviceIdentity::begin() {
    uint64_t mac = ESP.getEfuseMac();
    uint8_t* macBytes = reinterpret_cast<uint8_t*>(&mac);

    snprintf(_deviceId, sizeof(_deviceId), "CP-%02X%02X%02X%02X%02X%02X",
             macBytes[0], macBytes[1], macBytes[2],
             macBytes[3], macBytes[4], macBytes[5]);

    Logger::printf("DEVICE", "Device-ID:  %s", _deviceId);
    Logger::printf("DEVICE", "FW-Version: %s", FW_VERSION);
}

const char* DeviceIdentity::getDeviceId() {
    return _deviceId;
}

const char* DeviceIdentity::getFirmwareVersion() {
    return FW_VERSION;
}

uint32_t DeviceIdentity::getFlashSizeMB() {
    return ESP.getFlashChipSize() / (1024 * 1024);
}

uint32_t DeviceIdentity::getPsramSizeKB() {
    return ESP.getPsramSize() / 1024;
}
