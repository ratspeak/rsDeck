#pragma once

#include <Arduino.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"

// Boot-mode selection for the rsDeck dual-boot layout (launcher/standalone/rnode
// in ota_0/1/2). On the legacy single-app layout ota_0 is the running standalone,
// so re-arming the launcher is a harmless self-select.
namespace rs_deck {

enum class FirmwareMode : uint8_t {
    Launcher,
    Standalone,
    RNode,
};

struct SwitchResult {
    bool ok;
    const char* message;
};

inline esp_partition_subtype_t modeSubtype(FirmwareMode mode) {
    switch (mode) {
        case FirmwareMode::Launcher:
            return ESP_PARTITION_SUBTYPE_APP_OTA_0;
        case FirmwareMode::Standalone:
            return ESP_PARTITION_SUBTYPE_APP_OTA_1;
        case FirmwareMode::RNode:
            return ESP_PARTITION_SUBTYPE_APP_OTA_2;
    }
    return ESP_PARTITION_SUBTYPE_APP_OTA_0;
}

inline const esp_partition_t* modePartition(FirmwareMode mode) {
    return esp_partition_find_first(ESP_PARTITION_TYPE_APP, modeSubtype(mode), nullptr);
}

inline const char* modeName(FirmwareMode mode) {
    switch (mode) {
        case FirmwareMode::Launcher:
            return "Launcher";
        case FirmwareMode::Standalone:
            return "Standalone";
        case FirmwareMode::RNode:
            return "RNode";
    }
    return "Unknown";
}

inline SwitchResult setNextBoot(FirmwareMode mode) {
    const esp_partition_t* target = modePartition(mode);
    if (!target) {
        return {false, "target partition not found"};
    }

    esp_err_t err = esp_ota_set_boot_partition(target);
    if (err != ESP_OK) {
        return {false, esp_err_to_name(err)};
    }

    return {true, "boot partition updated"};
}

inline SwitchResult returnToLauncherNextBoot() {
    return setNextBoot(FirmwareMode::Launcher);
}

// Snake_case aliases for the launcher and RNode helper.
inline esp_partition_subtype_t mode_subtype(FirmwareMode mode) { return modeSubtype(mode); }
inline const esp_partition_t* mode_partition(FirmwareMode mode) { return modePartition(mode); }
inline const char* mode_name(FirmwareMode mode) { return modeName(mode); }
inline SwitchResult set_next_boot(FirmwareMode mode) { return setNextBoot(mode); }
inline SwitchResult return_to_launcher_next_boot() { return returnToLauncherNextBoot(); }

} // namespace rs_deck
