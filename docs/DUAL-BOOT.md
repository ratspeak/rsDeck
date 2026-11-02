# rsDeck Dual-Boot

This describes the retained rsDeck layout. For current firmware and installation,
see [Ratspeak Handheld](https://github.com/ratspeak/ratspeak-handheld) and the
[installation guide](https://docs.ratspeak.org/docs/hardware/flashing-firmware).

The dual-boot layout lets one flash carry three apps on the T-Deck Plus's 16MB
flash, with a boot-time chooser:

| Partition  | Slot  | Offset   | Size    | Contents |
|------------|-------|----------|---------|----------|
| launcher   | ota_0 | 0x10000  | 1MB     | Boot chooser UI |
| standalone | ota_1 | 0x110000 | 4MB     | Full rsDeck messenger |
| rnode      | ota_2 | 0x510000 | 3MB     | RNode (KISS over USB CDC + BLE) |
| littlefs   | —     | 0x810000 | ~7.9MB  | User data (same offset and size as single-app layout) |

The littlefs and coredump offsets and sizes match the legacy single-app table
(`partitions_16MB.csv`). The dual factory image ends before littlefs, but it
spans NVS and replaces the partition table and OTA selection data. Matching
littlefs offsets alone do not guarantee preservation of settings or data from
another firmware. Back up identity, messages and other user data before flashing;
a full-chip erase also removes littlefs.

## Boot flow

- The ESP-IDF second-stage bootloader reads `otadata` and starts the selected slot.
- The launcher (ota_0) shows Standalone/RNode, remembers the last choice in
  NVS (`rslaunch/last`), auto-boots it after 7 seconds, and writes the choice
  via `esp_ota_set_boot_partition()`.
- Both Standalone and RNode re-arm the launcher at every boot
  (`rs_deck::returnToLauncherNextBoot()`), so any reset returns to the chooser.
  On the legacy single-app layout that call self-selects ota_0 and is a no-op.

## Launcher controls

Trackball up/down + click, or keys: `W`/`S` select, `Enter` boot,
`R` = Standalone now, `N` = RNode now. Any input cancels auto-boot.

## RNode mode

The vendored RNode firmware (`make -C vendor/rnode_firmware firmware-tdeck`)
self-provisions EEPROM on first boot (PRODUCT_TDECK_V1 / MODEL_D9, default
radio config) and adopts the running firmware hash for this app image. BLE stays
connectable for already-bonded hosts; tap the on-screen Pair via BLE panel to
open a 30s pairing window for a new host. The display shows the standard RNode
UI (node address, waterfall, status) and the BLE pairing PIN during explicit
pairing; it blanks after 60s idle by default. Reset to return to the launcher.

## Build

Start with the [legacy build prerequisites](../README.md#build-from-source).

```bash
make prep-tdeck     # once: arduino-cli esp32 core + libs
make package        # dist/: rsdeck-full.zip, rsdeck-standalone.zip,
                    #        rsdeck-rnode.zip, rsdeck-*-m5launcher.bin
```

After backing up the device, `make flash PORT=/dev/cu.usbmodemXXXX` builds and
writes `build/rsdeck-full.bin` at offset `0x0`. Replace the port with your device
and close any app using it. The full image contains all three apps.
`rsdeck-standalone.zip` repackages the legacy single-app `rsdeck-merged.bin`
(old partition table, no launcher). The `*-m5launcher.bin` files are bare app
images for a compatible external launcher or a correctly sized OTA slot. Do not
write these bare applications at offset `0x0`. The RNode-only app image is
T-Deck-specific and self-provisioning; it is not a generic upstream `rnodeconf`
image.
