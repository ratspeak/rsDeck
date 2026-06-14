#pragma once

#include <Arduino.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"

namespace rs_cardputer_adv {

enum class FirmwareMode : uint8_t {
  Launcher,
  Ratcom,
  RNode,
};

struct SwitchResult {
  bool ok;
  const char *message;
};

inline esp_partition_subtype_t mode_subtype(FirmwareMode mode) {
  switch (mode) {
    case FirmwareMode::Launcher:
      return ESP_PARTITION_SUBTYPE_APP_OTA_0;
    case FirmwareMode::Ratcom:
      return ESP_PARTITION_SUBTYPE_APP_OTA_1;
    case FirmwareMode::RNode:
      return ESP_PARTITION_SUBTYPE_APP_OTA_2;
  }
  return ESP_PARTITION_SUBTYPE_APP_OTA_0;
}

inline const esp_partition_t *mode_partition(FirmwareMode mode) {
  return esp_partition_find_first(ESP_PARTITION_TYPE_APP, mode_subtype(mode), nullptr);
}

inline SwitchResult set_next_boot(FirmwareMode mode) {
  const esp_partition_t *target = mode_partition(mode);
  if (target == nullptr) {
    return {false, "target partition not found"};
  }

  esp_err_t err = esp_ota_set_boot_partition(target);
  if (err != ESP_OK) {
    return {false, esp_err_to_name(err)};
  }

  return {true, "boot partition updated"};
}

inline SwitchResult return_to_launcher_next_boot() {
  return set_next_boot(FirmwareMode::Launcher);
}

} // namespace rs_cardputer_adv
