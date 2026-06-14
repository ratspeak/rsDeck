# Cardputer Adv RNode Firmware Port

Base: upstream `markqvist/RNode_Firmware` tag `1.86` (`9b39b6c`).

Old reference branch: `ratspeak/rnode_firmware_ce` `cardputer-adv-support`
(`0cd0f64`). This repo ports the Cardputer Adv support to upstream firmware
instead of carrying the CE fork.

## Progress

- [x] Create local repo from upstream 1.86.
- [x] Add Cardputer Adv board/product/model IDs and pin map.
- [x] Add SX1262 custom SPI and Cardputer TCXO voltage handling.
- [x] Add M5 display rendering.
- [x] Add Cardputer keyboard shortcuts.
- [x] Add build/release targets.
- [x] Add Cardputer Adv first-boot EEPROM self-provisioning.
- [x] Add repo-local firmware hash writer and Makefile targets.
- [x] Add Cardputer Adv firmware-hash self-trust on first mismatched boot.
- [x] Keep Cardputer Adv RF idle until a USB/BLE host starts it.
- [x] Document Cardputer Adv USB CDC and BLE pairing behavior.
- [x] Port Ratcom Cardputer radio hardening: dedicated FSPI, cap RF switch,
      LDO regulator init, TCXO fallback, and TX-timeout radio recovery.
- [x] Build firmware target.
- [x] Document flashing/provisioning and remaining upstream tool gaps.

## Build Verification

Command run:

```sh
make firmware-cardputer_adv
make release-cardputer_adv
```

Result:

- Sketch uses `1571665` bytes, `47%` of program storage.
- Global variables use `99492` bytes, `30%` of dynamic memory.
- Build artifacts are written under `build/esp32.esp32.esp32s3/`.
- Release package is written to `Release/rnode_firmware_cardputer_adv.zip`.

## Hardware Notes

- Board: M5Stack Cardputer Adv, ESP32-S3, 8MB flash.
- Radio: M5Stack Cap LoRa-1262, SX1262, 850-950 MHz.
- LoRa pins: NSS `GPIO5`, IRQ/DIO1 `GPIO4`, RST `GPIO3`, BUSY `GPIO6`,
  SCK `GPIO40`, MOSI `GPIO14`, MISO `GPIO39`.
- TCXO: DIO3-controlled, 3.0V.
- DIO2 is used as the SX1262 RF switch control.
- SD is left disabled for this dumb RNode target because it shares the radio
  SPI bus and is not needed by host-controlled RNode operation.

## Tooling Gap

`rnodeconf` upstream does not currently know `PRODUCT_CARDPUTER_ADV=0xEC` or
`MODEL_EC=0xEC`. The firmware now self-provisions a missing or invalid EEPROM
identity block and seeds a default 915 MHz radio config, so a fresh Cardputer
Adv should not stop at the stock "run rnodeconf" config error. The seed profile
is build-time configurable with `CARDPUTER_ADV_DEFAULT_*` make variables, and
host software can still overwrite and persist normal RNode radio settings after
boot. The Cardputer Adv target does not start the seeded config on boot; it stays
in host-controlled mode and leaves the SX1262 idle until USB or BLE KISS starts
the radio.

USB CDC serial is enabled for the ESP32-S3 build and carries the normal RNode
KISS protocol. BLE is enabled by default; holding `p` or Enter/OK for three
seconds enters the pairing flow used by other RNode boards, holding `b` for
three seconds toggles BLE on or off, and pairing mode times out after 30 seconds
if no client connects.

The Cardputer Adv radio path follows Ratcom's hardened Cap LoRa-1262 handling:
LoRa uses a dedicated ESP32-S3 FSPI bus, the PI4IOE5V6408 RF antenna switch is
enabled before radio start, SX1262 init uses LDO regulator mode with the TCXO
kept as the TX/RX fallback clock, stale IRQ state is cleared when RX starts, and
TX timeout recovery restarts only the radio instead of rebooting the whole
device.

The repo-local `tools/cardputer_adv_firmware_hash.py` tool writes the expected
firmware hash with raw KISS. The firmware also self-stores the running ESP32 app
hash when the stored target hash does not match, so the Cardputer target does
not need upstream `rnodeconf` metadata to clear the stock firmware-corrupt
screen. First-class autoinstall and friendly `rnodeconf -i` output still need
matching Reticulum/rnodeconf metadata.
