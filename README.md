<div align="center">

# rsDeck <br> ⚠️ DEPRECATED - use [ratspeak-handheld](https://github.com/ratspeak/ratspeak-handheld) ⚠️

**Dual-mode Ratspeak firmware for the LilyGo T-Deck Plus.**

[![Status](https://img.shields.io/badge/status-beta-yellow.svg)](#install)
[![License](https://img.shields.io/badge/license-AGPL--3.0--or--later-blue.svg)](LICENSE)
[![Legacy version](https://img.shields.io/badge/legacy_version-2.0.2-success.svg)](https://github.com/ratspeak/rsDeck/releases)

[Ratspeak](https://github.com/ratspeak/Ratspeak) |
[Current docs](https://docs.ratspeak.org/docs/hardware/handheld-guide) |
[Legacy releases](https://github.com/ratspeak/rsDeck/releases) |
[rsReticulum](https://github.com/ratspeak/rsReticulum)

</div>

---

**Legacy repository.** Current firmware and development are in
[Ratspeak Handheld](https://github.com/ratspeak/ratspeak-handheld). The modes,
controls, radio defaults and build artifacts below describe this retained
repository; they are not the current handheld release instructions.

rsDeck turns a [LilyGo T-Deck Plus](https://www.lilygo.cc/products/t-deck-plus)
into a two-mode Reticulum handheld. Standalone mode is an on-device
Ratspeak/LXMF messenger. RNode mode makes the T-Deck a host-controlled radio
for Ratspeak, Sideband, or another Reticulum client over BLE or USB serial.

## Install

For current firmware, follow the
[handheld installation guide](https://docs.ratspeak.org/docs/hardware/flashing-firmware)
and choose **T-Deck Plus** in the
[Ratspeak web flasher](https://ratspeak.org/download.html). The current full
image is `tdeck-full.bin`.

Back up your identity and messages before changing firmware or layouts. Legacy
packages from this repository use different names and storage layouts; matching
a device name does not establish that an update will preserve its data.

## Modes

On boot, the launcher lets you choose:

- **Standalone**: a local Reticulum/LXMF messenger with identity management,
  contacts, peer discovery, messages, LoRa, and WiFi TCP access.
- **RNode**: a host-controlled RNode-style radio target for Ratspeak or other
  Reticulum clients over BLE or USB serial.

RNode mode self-provisions the T-Deck RNode product/model/default config and
running firmware hash on first boot, so users should not need a separate
`rnodeconf` setup step for the bundled release images.

## Basic Use

On first boot, Standalone mode generates a Reticulum identity and asks for a
display name. Your LXMF address is the 32-character hex string you share with
contacts.

- Tabs: Home, Chats, Contacts, Peers, Settings.
- Navigation: trackball movement and click/Enter.
- Announce: press the trackball or Enter on the Home tab.
- Add contacts: select a discovered peer, then open or save the chat.
- Send messages: open a chat, type, and press Enter.
- Delivery color: yellow while sending, green after delivery confirmation.
- Identity import: place one raw 64-byte Reticulum identity file under
  `/ratdeck/identity/` on the SD card, then use
  **Settings → Identity & Device → Import Identity**.
  Files named `import.identity` or `import.key` are preferred; otherwise rsDeck
  accepts exactly one non-reserved `.identity` or `.key` file in that folder.
  Ratspeak's encrypted `.rsi` backups are not imported on-device; from Ratspeak,
  export a Reticulum Identity File for rsDeck import.

## Radio Presets

`Long Fast` is the compiled-in default. All radio parameters can also be tuned
from Settings. Changes apply immediately.

| Preset | SF | BW | CR | TXP |
|---|---:|---:|---:|---:|
| Short Turbo | 7 | 500 kHz | 4/5 | 14 dBm |
| Short Fast | 7 | 250 kHz | 4/5 | 14 dBm |
| Short Slow | 8 | 250 kHz | 4/5 | 14 dBm |
| Medium Fast | 9 | 250 kHz | 4/5 | 17 dBm |
| Medium Slow | 10 | 250 kHz | 4/5 | 17 dBm |
| Long Turbo | 11 | 500 kHz | 4/8 | 22 dBm |
| **Long Fast** *(default)* | **11** | **250 kHz** | **4/5** | **22 dBm** |
| Long Moderate | 11 | 125 kHz | 4/8 | 22 dBm |

You are responsible for operating within local laws and radio regulations.

## WiFi Bridging

WiFi bridging is experimental. STA mode can connect to existing WiFi and reach
remote Reticulum nodes such as `rns.ratspeak.org:4242`.

AP mode exposes a local TCP endpoint for a nearby Reticulum host:

```ini
[[rsdeck]]
  type = TCPClientInterface
  target_host = 192.168.4.1
  target_port = 4242
```

This configuration describes the legacy firmware's access point. Join its WiFi
network first; use the address shown by the device if it differs from the default.

## Battery

The stock T-Deck Plus battery is read through a fixed ADC voltage-divider
ratio. Third-party/extended battery packs are commonly wired through a
different divider network on their sense line, which can make the reported
voltage (and therefore percentage) wrong even though the battery itself is
fine — e.g. reading ~69% when actually fully charged.

Tested and confirmed working with a
[makernova.io](https://makernova.io/) 8000mAh pack (JST 1.25 connector) in
[AlleyCat's extended T-Deck Plus case](https://www.printables.com/model/1345336)
(3D-printed, Printables #1345336).

If your battery percentage looks wrong, calibrate the ADC divider ratio
against a multimeter reading on the battery terminals (ideally with USB
disconnected, to avoid charge-line voltage bias):

```
b            # print raw ADC, computed voltage, divider ratio, percent
B<ratio>     # set + persist a new divider ratio, e.g. B1.8762
```

over the device's serial console (115200 baud). The firmware computes the
correct ratio for you from a live raw ADC reading + your multimeter value —
see the in-firmware `b` output for the exact formula. Settings persist across
reboots and firmware updates (stored alongside your identity/config, not
touched by a normal OTA/reflash).

The `Charge Above` and `Full Threshold` settings (Settings → Battery, hold
trackball click to reveal developer controls) are separate knobs:
`Charge Above` is the voltage above which the device assumes it's actively on
a charger (typically just under the LiPo absolute max of ~4.2V, not the
resting-full voltage); `Full Threshold` compensates the discharge curve for
your particular pack's typical resting-voltage sag under load. They're not
meant to be compared to each other directly.

## Build From Source

These commands build the retained legacy source. Install Git, Make, Python 3
with virtual-environment support, and Arduino CLI first. Use a fresh checkout:
packaging replaces files under `dist/`.

```bash
git clone https://github.com/ratspeak/rsDeck
cd rsDeck
git checkout 45b9c1a977385da96e9994d7a16966259d28e9f6
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install platformio esptool
make prep-tdeck
make package
```

After backing up the device, `make flash PORT=/dev/cu.usbmodemXXXX` builds and
writes the full legacy image at offset `0x0`. Replace the port with your device
and close any app using it. A factory write can replace settings; do not assume
that data from another firmware layout will survive.

Useful build targets:

```bash
make build-launcher      # launcher only
make build-standalone    # standalone messenger app
make build-rnode         # host-controlled RNode target
make full-image          # launcher + Standalone + RNode
make package             # release zips and launcher bins
```

Legacy artifacts are written to `dist/`:

```text
dist/rsdeck-full.zip
dist/rsdeck-standalone.zip
dist/rsdeck-rnode.zip
dist/rsdeck-standalone-m5launcher.bin
dist/rsdeck-rnode-m5launcher.bin
```

Each `.zip` contains a merged factory image and its legacy flasher manifest.
The `*-m5launcher.bin` files are bare applications for a compatible external
launcher or a correctly sized OTA slot. Do not write a bare application at
offset `0x0`.

## License

rsDeck standalone firmware, launcher, partition tables, and packaging tools are
licensed under the GNU Affero General Public License v3.0 or later. See
[LICENSE](LICENSE).

Vendored third-party code keeps its own license notices, including
`vendor/rnode_firmware/` and `lib/Crypto`.
