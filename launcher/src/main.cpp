// rsDeck boot launcher — picks Standalone or RNode at power-on.
// Runs from ota_0; writes the choice to otadata and restarts. Both target
// firmwares re-arm this launcher on boot, so any reset returns here.
// Input reuses the standalone HAL (Trackball, TouchInput) via shim includes.

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <LovyanGFX.hpp>

#include "RsDeckModeSwitch.h"
#include "config/BoardConfig.h"
#include "hal/Trackball.h"
#include "hal/TouchInput.h"

namespace {

constexpr uint16_t kBg = 0x0841;
constexpr uint16_t kPanel = 0x1082;
constexpr uint16_t kText = 0xF7BE;
constexpr uint16_t kMuted = 0x8C71;
constexpr uint16_t kAccent = 0x06D7;
constexpr uint16_t kWarn = 0xFBA0;
constexpr uint32_t kAutoBootMs = 7000;
constexpr char kPrefsNamespace[] = "rslaunch";
constexpr char kLastChoiceKey[] = "last";

// Option card geometry (shared by renderer and touch hit-testing)
constexpr int16_t kCardX = 24;
constexpr int16_t kCardW = 272;
constexpr int16_t kCardH = 52;
constexpr int16_t kCard1Y = 66;
constexpr int16_t kCard2Y = 126;

class LGFX_TDeckLauncher : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;

public:
  LGFX_TDeckLauncher() {
    auto cfg_bus = _bus.config();
    cfg_bus.spi_host = SPI2_HOST;
    cfg_bus.spi_mode = 0;
    cfg_bus.freq_write = 27000000;
    cfg_bus.freq_read = 16000000;
    cfg_bus.pin_sclk = SPI_SCK;
    cfg_bus.pin_miso = SPI_MISO;
    cfg_bus.pin_mosi = SPI_MOSI;
    cfg_bus.pin_dc = TFT_DC;
    _bus.config(cfg_bus);
    _panel.setBus(&_bus);

    // Panel is natively 240x320 portrait; rotated to 320x240 in setup().
    auto cfg_panel = _panel.config();
    cfg_panel.pin_cs = TFT_CS;
    cfg_panel.pin_rst = -1;
    cfg_panel.panel_width = 240;
    cfg_panel.panel_height = 320;
    cfg_panel.invert = true;
    cfg_panel.rgb_order = false;
    cfg_panel.memory_width = 240;
    cfg_panel.memory_height = 320;
    _panel.config(cfg_panel);

    auto cfg_light = _light.config();
    cfg_light.pin_bl = TFT_BL;
    cfg_light.invert = false;
    cfg_light.freq = 12000;
    cfg_light.pwm_channel = 0;
    _light.config(cfg_light);
    _panel.setLight(&_light);

    setPanel(&_panel);
  }
};

LGFX_TDeckLauncher display;
Trackball trackball;
TouchInput touch;

enum class Choice : uint8_t {
  Standalone = 0,
  RNode = 1,
};

Choice selected = Choice::Standalone;
uint32_t bootStarted = 0;
uint32_t lastRemain = UINT32_MAX;
bool booting = false;
bool autoBootEnabled = true;

int16_t scrollAccum = 0;
uint32_t lastNavMove = 0;
bool touchWasDown = false;
int16_t touchLastX = 0;
int16_t touchLastY = 0;

uint8_t choiceValue(Choice choice) {
  return choice == Choice::RNode ? 1 : 0;
}

Choice choiceFromValue(uint8_t value) {
  return value == 1 ? Choice::RNode : Choice::Standalone;
}

Choice loadLastChoice() {
  Preferences prefs;
  Choice choice = Choice::Standalone;
  if (prefs.begin(kPrefsNamespace, true)) {
    choice = choiceFromValue(prefs.getUChar(kLastChoiceKey, choiceValue(choice)));
    prefs.end();
  }
  return choice;
}

void saveLastChoice(Choice choice) {
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    prefs.putUChar(kLastChoiceKey, choiceValue(choice));
    prefs.end();
  }
}

void drawOption(int16_t y, const char *title, const char *subtitle, bool active) {
  uint16_t fill = active ? kAccent : kPanel;
  uint16_t fg = active ? TFT_BLACK : kText;
  uint16_t sub = active ? 0x2104 : kMuted;

  display.fillRoundRect(kCardX, y, kCardW, kCardH, 6, fill);
  display.setTextColor(fg, fill);
  display.setTextSize(2);
  display.setCursor(kCardX + 16, y + 9);
  display.print(title);
  display.setTextColor(sub, fill);
  display.setTextSize(1);
  display.setCursor(kCardX + 16, y + 32);
  display.print(subtitle);
}

uint32_t remainingSeconds() {
  uint32_t elapsed = millis() - bootStarted;
  if (elapsed >= kAutoBootMs) {
    return 0;
  }
  return (kAutoBootMs - elapsed + 999) / 1000;
}

void drawCountdown(bool force = false) {
  if (!autoBootEnabled) {
    display.fillRect(280, 8, 34, 24, kBg);
    lastRemain = UINT32_MAX;
    return;
  }

  uint32_t remain = remainingSeconds();
  if (!force && remain == lastRemain) {
    return;
  }
  lastRemain = remain;

  display.fillRect(280, 8, 34, 24, kBg);
  display.fillRoundRect(286, 9, 26, 22, 6, kPanel);
  display.setTextSize(2);
  display.setTextColor(kText, kPanel);
  display.setCursor(remain >= 10 ? 289 : 294, 13);
  display.print(static_cast<unsigned long>(remain));
}

void drawScreen() {
  display.fillScreen(kBg);

  display.setTextSize(3);
  display.setTextColor(kText, kBg);
  display.setCursor(24, 14);
  display.print("rsDeck");

  display.setTextSize(1);
  display.setTextColor(kMuted, kBg);
  display.setCursor(26, 44);
  display.print("T-Deck Plus");

  drawOption(kCard1Y, "Standalone", "On-device messenger", selected == Choice::Standalone);
  drawOption(kCard2Y, "RNode", "BLE / USB radio", selected == Choice::RNode);

  display.setTextSize(1);
  display.setTextColor(kMuted, kBg);
  display.setCursor(24, 196);
  display.print("Trackball or W/S select, click/Enter boot");
  display.setCursor(24, 212);
  display.print("Touch: tap card, tap again to boot   R/N quick");

  drawCountdown(true);
}

void selectChoice(Choice choice) {
  if (selected == choice) {
    return;
  }
  selected = choice;
  drawScreen();
}

void pauseAutoBoot() {
  if (!autoBootEnabled) {
    return;
  }
  autoBootEnabled = false;
  drawCountdown(true);
}

void showBooting(const char *label, bool rnode) {
  booting = true;
  display.fillScreen(kBg);
  display.setTextSize(3);
  display.setTextColor(kAccent, kBg);
  display.setCursor(28, 70);
  display.print(label);
  display.setTextSize(1);
  display.setTextColor(kMuted, kBg);
  display.setCursor(30, 110);
  display.print("Starting...");
  if (rnode) {
    display.setCursor(30, 134);
    display.print("Reset the device to return here.");
  }
}

void showError(const char *message) {
  booting = false;
  display.fillScreen(kBg);
  display.setTextSize(2);
  display.setTextColor(kWarn, kBg);
  display.setCursor(24, 60);
  display.print("Boot error");
  display.setTextSize(1);
  display.setTextColor(kText, kBg);
  display.setCursor(24, 96);
  display.print(message);
}

void startChoice(Choice choice) {
  using namespace rs_deck;

  FirmwareMode mode = choice == Choice::Standalone ? FirmwareMode::Standalone : FirmwareMode::RNode;
  Serial.printf("[LAUNCHER] booting %s\n", mode_name(mode));
  showBooting(mode_name(mode), mode == FirmwareMode::RNode);
  SwitchResult result = set_next_boot(mode);
  if (!result.ok) {
    Serial.printf("[LAUNCHER] boot switch failed: %s\n", result.message);
    showError(result.message);
    return;
  }
  saveLastChoice(choice);
  delay(mode == FirmwareMode::RNode ? 1500 : 50);
  esp_restart();
}

void handleKey(char key) {
  pauseAutoBoot();

  if (key == '\r' || key == '\n') {
    startChoice(selected);
    return;
  }
  if (key == 'w' || key == 'W' || key == ';') {
    selectChoice(Choice::Standalone);
    return;
  }
  if (key == 's' || key == 'S' || key == '.') {
    selectChoice(Choice::RNode);
    return;
  }
  if (key == 'r' || key == 'R') {
    startChoice(Choice::Standalone);
    return;
  }
  if (key == 'n' || key == 'N') {
    startChoice(Choice::RNode);
    return;
  }
}

char pollKeyboard() {
  Wire.requestFrom((uint8_t)KB_I2C_ADDR, (uint8_t)1);
  if (Wire.available()) {
    char key = (char)Wire.read();
    if (key != 0) {
      return key;
    }
  }
  return 0;
}

void pollTrackball() {
  trackball.update();

  if (trackball.wasClicked()) {
    pauseAutoBoot();
    startChoice(selected);
    return;
  }

  int8_t dy = trackball.lastDeltaY();
  if (dy != 0) {
    pauseAutoBoot();
    scrollAccum += dy;
    // Two raw counts per selection step, rate-limited so a flick moves once.
    uint32_t now = millis();
    if (now - lastNavMove > 150) {
      if (scrollAccum <= -2) {
        lastNavMove = now;
        scrollAccum = 0;
        selectChoice(Choice::Standalone);
      } else if (scrollAccum >= 2) {
        lastNavMove = now;
        scrollAccum = 0;
        selectChoice(Choice::RNode);
      }
    }
  }
}

void pollTouch() {
  touch.update();

  if (touch.isTouched()) {
    touchWasDown = true;
    touchLastX = touch.x();
    touchLastY = touch.y();
    return;
  }

  if (!touchWasDown) {
    return;
  }
  touchWasDown = false;

  // Tap on release: select a card, tap the selected card again to boot.
  pauseAutoBoot();
  if (touchLastX >= kCardX && touchLastX < kCardX + kCardW) {
    if (touchLastY >= kCard1Y && touchLastY < kCard1Y + kCardH) {
      if (selected == Choice::Standalone) {
        startChoice(Choice::Standalone);
      } else {
        selectChoice(Choice::Standalone);
      }
    } else if (touchLastY >= kCard2Y && touchLastY < kCard2Y + kCardH) {
      if (selected == Choice::RNode) {
        startChoice(Choice::RNode);
      } else {
        selectChoice(Choice::RNode);
      }
    }
  }
}

} // namespace

void setup() {
  // Peripheral power must come up before anything touches the display or I2C.
  pinMode(BOARD_POWER_PIN, OUTPUT);
  digitalWrite(BOARD_POWER_PIN, HIGH);
  delay(50);

  // Park the other chip selects on the shared SPI bus.
  pinMode(LORA_CS, OUTPUT);
  digitalWrite(LORA_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  trackball.begin();
  touch.begin();

  display.init();
  display.setRotation(1);
  display.setBrightness(200);

  selected = loadLastChoice();
  bootStarted = millis();
  Serial.printf("[LAUNCHER] rsDeck launcher up, last choice: %s\n",
                selected == Choice::RNode ? "RNode" : "Standalone");
  drawScreen();
}

void loop() {
  if (booting) {
    delay(20);
    return;
  }

  char key = pollKeyboard();
  if (key != 0) {
    handleKey(key);
  }

  pollTrackball();
  pollTouch();
  drawCountdown();

  if (autoBootEnabled && millis() - bootStarted >= kAutoBootMs) {
    startChoice(selected);
    return;
  }

  delay(5);
}
