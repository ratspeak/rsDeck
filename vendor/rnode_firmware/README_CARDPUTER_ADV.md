# Cardputer Adv RNode Firmware

This repo is a local Cardputer Adv firmware port based on upstream
`markqvist/RNode_Firmware` tag `1.86`, not the RNode Firmware CE fork.

It targets the M5Stack Cardputer Adv with the M5Stack Cap LoRa-1262 as a
regular host-controlled RNode.

## Build

One-time dependency setup:

```sh
make prep-cardputer_adv
```

Build the firmware:

```sh
make firmware-cardputer_adv
```

The first-boot radio seed can be overridden at build time without editing
source. For example:

```sh
make firmware-cardputer_adv CARDPUTER_ADV_DEFAULT_FREQ=868000000UL
```

The build target stages a temporary Arduino sketch under `build/` because
Arduino CLI requires the sketch folder name to match `RNode_Firmware.ino`.
Compiled artifacts are emitted to:

```text
build/esp32.esp32.esp32s3/
```

Create a release zip:

```sh
make release-cardputer_adv
```

## Flash

With the Cardputer connected in bootloader mode:

```sh
make upload-cardputer_adv port=/dev/ttyACM0
```

On macOS the port is usually `/dev/cu.usbmodem*`.

The upload target writes the ESP32-S3 image and then writes the expected RNode
firmware hash over KISS. This avoids the stock firmware-corrupt screen after a
successful flash.

If firmware is already flashed and the display only needs its hash repaired:

```sh
make set-running-hash-cardputer_adv port=/dev/ttyACM0
```

That reads the running app hash from the device and writes it back as the trusted
target hash. To write the hash for a freshly built local binary instead:

```sh
make set-hash-cardputer_adv port=/dev/ttyACM0
```

## Host Connection

Cardputer Adv boots as a host-controlled RNode. First-boot provisioning writes a
valid EEPROM identity and a default radio profile, but the SX1262 remains idle
until a USB or BLE host sends the normal RNode radio-on command.

- USB CDC serial is enabled by the Cardputer Adv build flags and carries the
  normal RNode KISS protocol.
- BLE is enabled by default. Hold `p` or Enter/OK for three seconds to enter
  pairing mode, or hold `b` for three seconds to toggle BLE on or off. Pairing
  mode times out after 30 seconds if no client connects.
- The display's LoRa icon is dim while the radio is idle. The waterfall only
  updates after host software starts the radio.

## Hardware Map

- ESP32-S3, 8 MB flash
- SX1262 LoRa radio
- NSS: `GPIO5`
- IRQ/DIO1: `GPIO4`
- RST: `GPIO3`
- BUSY: `GPIO6`
- SCK: `GPIO40`
- MOSI: `GPIO14`
- MISO: `GPIO39`
- TCXO: DIO3-controlled, 3.0 V
- DIO2: RF switch control
- Cap LoRa-1262 antenna path: PI4IOE5V6408 I2C expander `0x43`, P0 driven high
- Radio SPI: dedicated ESP32-S3 FSPI bus at 8 MHz

SD is intentionally disabled for this dumb RNode target. It shares the LoRa SPI
bus and is not needed for host-controlled operation.

## Provisioning Note

The firmware self-provisions Cardputer Adv EEPROM on first boot if the hardware
identity block is missing or invalid. It writes:

- product `PRODUCT_CARDPUTER_ADV=0xEC`
- model `MODEL_EC=0xEC`
- hardware revision `0x01`
- a serial derived from the ESP32-S3 efuse MAC
- the required RNode info checksum and lock byte

If no radio config exists, it also writes a default US 915 MHz profile:

- frequency `915000000`
- bandwidth `250000`
- spreading factor `11`
- coding rate `5`
- transmit power `14`

Host clients can overwrite the radio config later through normal RNode KISS
commands; once a config is present, the firmware does not replace it on later
boots. The saved profile is not started automatically on Cardputer Adv; it is
used when host software starts the radio without first sending replacement
settings. The Cardputer Adv target is for the 850-950 MHz SX1262 LoRa cap, so 868
MHz and 915 MHz are valid software profiles for the supported hardware. 433 MHz
requires a 433 MHz radio module and should be handled as a separate hardware
target. Upstream `rnodeconf` does not currently have friendly metadata for
product/model `0xEC`, so first-class autoinstall and friendly `rnodeconf -i`
output still require matching tooling changes outside this firmware repo.

## Firmware Hash

RNode firmware validates the running app partition against a SHA-256 hash stored
in EEPROM. The Cardputer Adv build self-stores the running app hash if the
stored target hash does not match, so a fresh flash can recover from the stock
firmware-corrupt state without upstream `rnodeconf` support.

`tools/cardputer_adv_firmware_hash.py` also implements the minimal KISS commands
needed by this repo:

- compute the ESP32 app hash from `RNode_Firmware.ino.bin`
- write `CMD_FW_HASH`
- optionally read the device's current running hash and trust it
- optionally reset the device after the write

It mirrors the hash calculation used by `rnodeconf` and the Ratspeak web
flasher: if an ESP32 binary ends with a matching SHA-256 trailer, that trailer is
the partition hash; otherwise the whole file is hashed.
