// Full-screen RNode UI for the LilyGo T-Deck (rsDeck dual-boot build).
// Renders into a PSRAM-backed 320x240 canvas pushed to the ST7789, replacing
// the generic 64x128 OLED layout. Included by Display.h for BOARD_TDECK only.
// Touch (GT911) toggles the display for battery savings; buttons and BT
// events wake it via the standard display_unblank() path.

#ifndef TDECK_UI_H
#define TDECK_UI_H

void display_unblank();
void bt_enable_pairing();
void bt_disable_pairing();
void bt_debond_all();
int bt_bond_count();
void di_conf_save(uint8_t dint);
extern uint32_t bt_pairing_started;
#define TD_PAIRING_WINDOW_MS 30000

// Ratspeak-style dark palette (RGB565)
#define TD_CLR_BG            0x0041
#define TD_CLR_BG_PANEL      0x1082
#define TD_CLR_BG_STATUSBAR  0x0861
#define TD_CLR_BORDER        0x2945
#define TD_CLR_TEXT_PRIMARY  0xE73C
#define TD_CLR_TEXT_SECONDARY 0xAD75
#define TD_CLR_TEXT_DIM      0x630C
#define TD_CLR_ACCENT        0x4EBF
#define TD_CLR_GREEN         0x070D
#define TD_CLR_WARN          0xFD20
#define TD_CLR_ERROR         0xF8A2

#define TD_W 320
#define TD_H 240
#define TD_BAR_H 24
#define TD_FOOT_Y 226
#define TD_CONTENT_Y 30
#define TD_LEFT_W 206
#define TD_WF_X 212
#define TD_WF_W 107
#define TD_WF_Y 30

#define TD_BTN_X (TD_WF_X + 5)
#define TD_BTN_W (TD_WF_W - 10)
#define TD_PAIR_BTN_H 42
#define TD_SETTINGS_BTN_H TD_PAIR_BTN_H
#define TD_PANEL_H (TD_FOOT_Y - TD_WF_Y - 4)
#define TD_ONLINE_SETTINGS_BTN_Y (TD_FOOT_Y - TD_SETTINGS_BTN_H - 4)
#define TD_WF_H (TD_ONLINE_SETTINGS_BTN_Y - TD_WF_Y - 8)
#define TD_WF_SIZE TD_WF_H
#define TD_PAIR_BTN_Y (TD_WF_Y + 42)
#define TD_SETTINGS_BTN_Y (TD_PAIR_BTN_Y + TD_PAIR_BTN_H + 10)
#define TD_CANCEL_BTN_Y (TD_WF_Y + 130)

#define TD_SETTINGS_BACK_X 252
#define TD_SETTINGS_BACK_Y 32
#define TD_SETTINGS_BACK_W 58
#define TD_SETTINGS_BACK_H 28
#define TD_SETTINGS_MINUS_X 18
#define TD_SETTINGS_MINUS_Y 88
#define TD_SETTINGS_STEP_W 36
#define TD_SETTINGS_STEP_H 30
#define TD_SETTINGS_PLUS_X 266
#define TD_SETTINGS_PLUS_Y 88
#define TD_SETTINGS_BAR_X 66
#define TD_SETTINGS_BAR_Y 99
#define TD_SETTINGS_BAR_W 188
#define TD_SETTINGS_BAR_H 8
#define TD_SETTINGS_ABOUT_X 18
#define TD_SETTINGS_ABOUT_Y 128
#define TD_SETTINGS_ABOUT_W 132
#define TD_SETTINGS_ABOUT_H 34
#define TD_SETTINGS_SLEEP_X 166
#define TD_SETTINGS_SLEEP_Y 128
#define TD_SETTINGS_SLEEP_W 136
#define TD_SETTINGS_SLEEP_H 34
#define TD_SETTINGS_FORGET_X 18
#define TD_SETTINGS_FORGET_Y 174
#define TD_SETTINGS_FORGET_W 284
#define TD_SETTINGS_FORGET_H 34
#define TD_ABOUT_CLOSE_X 126
#define TD_ABOUT_CLOSE_Y 178
#define TD_ABOUT_CLOSE_W 68
#define TD_ABOUT_CLOSE_H 32
#define TD_BRIGHTNESS_MIN_PCT 5
#define TD_BRIGHTNESS_STEP_PCT 5

#ifndef RSDECK_VERSION_MAJOR
  #define RSDECK_VERSION_MAJOR 0
#endif
#ifndef RSDECK_VERSION_MINOR
  #define RSDECK_VERSION_MINOR 0
#endif
#ifndef RSDECK_VERSION_PATCH
  #define RSDECK_VERSION_PATCH 0
#endif

static const uint16_t td_wf_palette[9] = {
  0x0000, 0x0011, 0x001F, 0x07FF,
  0x07E0, 0xFFE0, 0xFBE0, 0xF800, 0xFFFF
};

GFXcanvas16* td_canvas = nullptr;
bool td_ui_ready = false;
int td_waterfall[TD_WF_SIZE];
int td_waterfall_head = 0;
uint8_t td_charge_tick = 0;

enum TdView : uint8_t {
  TD_VIEW_MAIN = 0,
  TD_VIEW_SETTINGS = 1,
  TD_VIEW_ABOUT = 2
};

TdView td_view = TD_VIEW_MAIN;
uint8_t td_settings_brightness_pct = 80;
uint32_t td_settings_forget_until = 0;
uint32_t td_settings_status_until = 0;
const char* td_settings_status_msg = nullptr;

// --- GT911 touch (poll-only, tap toggles display power) ---
#define TD_TOUCH_SDA 18
#define TD_TOUCH_SCL 8
uint8_t td_touch_addr = 0;
bool td_touch_down = false;
int16_t td_touch_x = 0;
int16_t td_touch_y = 0;
uint32_t td_last_touch_poll = 0;

// Calibrated GT911 raw bounds, identical to the standalone firmware HAL.
#define TD_TOUCH_X_MIN 10
#define TD_TOUCH_Y_MIN 8
#define TD_TOUCH_X_MAX 313
#define TD_TOUCH_Y_MAX 243

void td_touch_init() {
  Wire.begin(TD_TOUCH_SDA, TD_TOUCH_SCL, 400000);
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) { td_touch_addr = 0x5D; return; }
  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) { td_touch_addr = 0x14; }
}

void td_touch_clear_flag() {
  Wire.beginTransmission(td_touch_addr);
  Wire.write(0x81); Wire.write(0x4E); Wire.write(0x00);
  Wire.endTransmission();
}

bool td_touch_read(int16_t* out_x, int16_t* out_y) {
  if (td_touch_addr == 0) return false;
  Wire.beginTransmission(td_touch_addr);
  Wire.write(0x81); Wire.write(0x4E);
  if (Wire.endTransmission() != 0) return false;
  Wire.requestFrom(td_touch_addr, (uint8_t)1);
  if (!Wire.available()) return false;
  uint8_t status = Wire.read();
  uint8_t count = status & 0x0F;
  if (!(status & 0x80) || count == 0) {
    if (status & 0x80) td_touch_clear_flag();
    return false;
  }

  Wire.beginTransmission(td_touch_addr);
  Wire.write(0x81); Wire.write(0x4F);
  if (Wire.endTransmission() != 0) { td_touch_clear_flag(); return false; }
  Wire.requestFrom(td_touch_addr, (uint8_t)5);
  if (Wire.available() < 5) { td_touch_clear_flag(); return false; }
  Wire.read(); // track id
  int16_t ry = Wire.read() | (Wire.read() << 8);
  int16_t rx = Wire.read() | (Wire.read() << 8);
  td_touch_clear_flag();

  ry = TD_H - 1 - ry;
  rx = (rx - TD_TOUCH_X_MIN) * (TD_W - 1) / (TD_TOUCH_X_MAX - TD_TOUCH_X_MIN);
  ry = (ry - TD_TOUCH_Y_MIN) * (TD_H - 1) / (TD_TOUCH_Y_MAX - TD_TOUCH_Y_MIN);
  if (rx < 0) rx = 0;
  if (rx >= TD_W) rx = TD_W - 1;
  if (ry < 0) ry = 0;
  if (ry >= TD_H) ry = TD_H - 1;
  *out_x = rx;
  *out_y = ry;
  return true;
}

bool td_pair_button_visible() {
  return !radio_online && bt_state != BT_STATE_CONNECTED && bt_state != BT_STATE_PAIRING;
}

bool td_settings_button_visible() {
  return bt_state != BT_STATE_PAIRING;
}

bool td_hit(int16_t x, int16_t y, int rx, int ry, int rw, int rh) {
  return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

uint8_t td_clamp_brightness_pct(uint8_t pct) {
  if (pct < TD_BRIGHTNESS_MIN_PCT) return TD_BRIGHTNESS_MIN_PCT;
  if (pct > 100) return 100;
  uint8_t snapped = ((pct + (TD_BRIGHTNESS_STEP_PCT / 2)) / TD_BRIGHTNESS_STEP_PCT) * TD_BRIGHTNESS_STEP_PCT;
  if (snapped < TD_BRIGHTNESS_MIN_PCT) return TD_BRIGHTNESS_MIN_PCT;
  if (snapped > 100) return 100;
  return snapped;
}

uint8_t td_intensity_to_pct(uint8_t intensity) {
  if (intensity == 0 || intensity == 0xFF) return TDECK_DISPLAY_INTENSITY_DEFAULT;
  return td_clamp_brightness_pct(intensity);
}

uint8_t td_pct_to_intensity(uint8_t pct) {
  return td_clamp_brightness_pct(pct);
}

void td_settings_set_status(const char* msg, uint16_t duration_ms = 1800) {
  td_settings_status_msg = msg;
  td_settings_status_until = millis() + duration_ms;
}

void td_settings_load_brightness() {
  uint8_t intensity = display_intensity;
  if (intensity == 0 && display_unblank_intensity != 0) intensity = display_unblank_intensity;
  td_settings_brightness_pct = td_intensity_to_pct(intensity);
}

void td_settings_preview_brightness(uint8_t pct) {
  td_settings_brightness_pct = td_clamp_brightness_pct(pct);
  display_intensity = td_pct_to_intensity(td_settings_brightness_pct);
  display_unblank_intensity = 0;
  di_conf_save(display_intensity);
  display_blank_frame_drawn = false;
  display_unblank();
}

void td_open_settings() {
  td_view = TD_VIEW_SETTINGS;
  td_settings_forget_until = 0;
  td_settings_status_msg = nullptr;
  td_settings_load_brightness();
  display_unblank();
  last_disp_update = 0;
}

void td_sleep_display() {
  uint8_t current = display_intensity;
  if (current == 0 || current == 0xFF) current = TDECK_DISPLAY_INTENSITY_DEFAULT;
  display_unblank_intensity = current;
  display_intensity = 0;
  display_blanked = true;
  display_blank_frame_drawn = false;
  td_view = TD_VIEW_MAIN;
  last_unblank_event = millis() - display_blanking_timeout - 1;
  last_disp_update = 0;
}

bool td_handle_settings_touch(int16_t x, int16_t y) {
  if (td_hit(x, y, TD_SETTINGS_BACK_X, TD_SETTINGS_BACK_Y, TD_SETTINGS_BACK_W, TD_SETTINGS_BACK_H)) {
    td_view = TD_VIEW_MAIN;
    td_settings_forget_until = 0;
    display_unblank();
    return true;
  }

  if (td_hit(x, y, TD_SETTINGS_MINUS_X, TD_SETTINGS_MINUS_Y, TD_SETTINGS_STEP_W, TD_SETTINGS_STEP_H)) {
    uint8_t next = td_settings_brightness_pct;
    if (next > TD_BRIGHTNESS_MIN_PCT + TD_BRIGHTNESS_STEP_PCT) next -= TD_BRIGHTNESS_STEP_PCT;
    else next = TD_BRIGHTNESS_MIN_PCT;
    td_settings_preview_brightness(next);
    return true;
  }

  if (td_hit(x, y, TD_SETTINGS_PLUS_X, TD_SETTINGS_PLUS_Y, TD_SETTINGS_STEP_W, TD_SETTINGS_STEP_H)) {
    uint8_t next = td_settings_brightness_pct;
    if (next < 100 - TD_BRIGHTNESS_STEP_PCT) next += TD_BRIGHTNESS_STEP_PCT;
    else next = 100;
    td_settings_preview_brightness(next);
    return true;
  }

  if (td_hit(x, y, TD_SETTINGS_ABOUT_X, TD_SETTINGS_ABOUT_Y, TD_SETTINGS_ABOUT_W, TD_SETTINGS_ABOUT_H)) {
    td_view = TD_VIEW_ABOUT;
    display_unblank();
    return true;
  }

  if (td_hit(x, y, TD_SETTINGS_SLEEP_X, TD_SETTINGS_SLEEP_Y, TD_SETTINGS_SLEEP_W, TD_SETTINGS_SLEEP_H)) {
    td_sleep_display();
    return true;
  }

  if (td_hit(x, y, TD_SETTINGS_FORGET_X, TD_SETTINGS_FORGET_Y, TD_SETTINGS_FORGET_W, TD_SETTINGS_FORGET_H)) {
    if ((int32_t)(td_settings_forget_until - millis()) > 0) {
      bt_debond_all();
      bt_disable_pairing();
      td_settings_forget_until = 0;
      td_settings_set_status("BLE bonds removed", 2200);
    } else {
      td_settings_forget_until = millis() + 5000;
      td_settings_set_status("Tap Forget again", 5000);
    }
    display_unblank();
    return true;
  }

  return false;
}

bool td_handle_about_touch(int16_t x, int16_t y) {
  if (td_hit(x, y, TD_ABOUT_CLOSE_X, TD_ABOUT_CLOSE_Y, TD_ABOUT_CLOSE_W, TD_ABOUT_CLOSE_H)) {
    td_view = TD_VIEW_SETTINGS;
    display_unblank();
    return true;
  }

  return false;
}

void td_poll_touch() {
  if (millis() - td_last_touch_poll < 50) return;
  td_last_touch_poll = millis();

  int16_t x, y;
  bool down = td_touch_read(&x, &y);
  if (down) {
    td_touch_down = true;
    td_touch_x = x;
    td_touch_y = y;
    return;
  }
  if (!td_touch_down) return;
  td_touch_down = false;

  // Tap released. Taps only wake or interact — the screen sleeps on the
  // inactivity timeout alone, never from a tap.
  if (display_blanked) {
    display_unblank();
    td_view = TD_VIEW_MAIN;
    last_disp_update = 0;
    return;
  }

  if (td_view == TD_VIEW_SETTINGS) {
    if (!td_handle_settings_touch(td_touch_x, td_touch_y)) {
      display_unblank();
    }
    last_disp_update = 0;
    return;
  }

  if (td_view == TD_VIEW_ABOUT) {
    if (!td_handle_about_touch(td_touch_x, td_touch_y)) {
      display_unblank();
    }
    last_disp_update = 0;
    return;
  }

  int settings_y = radio_online ? TD_ONLINE_SETTINGS_BTN_Y : TD_SETTINGS_BTN_Y;
  if (td_pair_button_visible() && td_hit(td_touch_x, td_touch_y, TD_BTN_X, TD_PAIR_BTN_Y, TD_BTN_W, TD_PAIR_BTN_H)) {
    bt_enable_pairing();           // Auto-disarms via BT_PAIRING_TIMEOUT.
  } else if (bt_state == BT_STATE_PAIRING && td_hit(td_touch_x, td_touch_y, TD_BTN_X, TD_CANCEL_BTN_Y, TD_BTN_W, TD_SETTINGS_BTN_H)) {
    bt_disable_pairing();          // tap again to cancel
  } else if (td_settings_button_visible() && td_hit(td_touch_x, td_touch_y, TD_BTN_X, settings_y, TD_BTN_W, TD_SETTINGS_BTN_H)) {
    td_open_settings();
  } else {
    display_unblank();             // count as activity, extend the timeout
  }
  last_disp_update = 0;
}

// T-Deck keyboard (ESP32-C3 @ I2C 0x55, single translated byte per read).
// Any keypress wakes the display / extends the inactivity timeout.
#define TD_KB_ADDR 0x55
uint32_t td_last_kb_poll = 0;

void td_poll_keyboard() {
  if (millis() - td_last_kb_poll < 50) return;
  td_last_kb_poll = millis();

  Wire.requestFrom((uint8_t)TD_KB_ADDR, (uint8_t)1);
  if (Wire.available() && Wire.read() != 0) {
    display_unblank();
    last_disp_update = 0;
  }
}

void td_ui_init() {
  if (td_ui_ready) return;
  display.setRotation(3);
  display.fillScreen(TD_CLR_BG);
  td_canvas = new GFXcanvas16(TD_W, TD_H);
  for (int i = 0; i < TD_WF_SIZE; i++) td_waterfall[i] = 0;
  td_touch_init();
  display_blanking_enabled = true;
  td_ui_ready = true;
}

void td_push_frame() {
  if (td_canvas && td_canvas->getBuffer()) {
    display.drawRGBBitmap(0, 0, td_canvas->getBuffer(), TD_W, TD_H);
  }
}

uint16_t td_load_color(float percent) {
  if (percent < 33.0f) return TD_CLR_GREEN;
  if (percent < 66.0f) return 0xFBE0;
  return 0xF800;
}

void td_draw_gradient_bar(int x, int y, int w, int h, float percent) {
  if (percent > 100.0f) percent = 100.0f;
  if (percent < 0.0f) percent = 0.0f;
  int fill_w = (int)((percent / 100.0f) * (w - 2));
  uint16_t fill_clr = TD_CLR_GREEN;
  if (percent < 33.0f) fill_clr = 0xF800;
  else if (percent < 66.0f) fill_clr = 0xFBE0;
  td_canvas->drawRect(x, y, w, h, TD_CLR_BORDER);
  if (fill_w > 0) td_canvas->fillRect(x + 1, y + 1, fill_w, h - 2, fill_clr);
}

int td_text_width(const char* label, int text_size = 1) {
  int len = 0;
  while (label && label[len] != '\0') len++;
  return len * 6 * text_size;
}

void td_draw_centered_text(const char* label, int x, int y, int w, int text_size, uint16_t colour) {
  td_canvas->setTextSize(text_size);
  td_canvas->setTextColor(colour);
  int tw = td_text_width(label, text_size);
  int tx = x + ((w - tw) / 2);
  if (tx < x + 2) tx = x + 2;
  td_canvas->setCursor(tx, y);
  td_canvas->print(label);
}

void td_draw_button(int x, int y, int w, int h, const char* label, uint16_t border, uint16_t text, bool armed = false) {
  uint16_t fill = armed ? 0x3000 : TD_CLR_BG_PANEL;
  td_canvas->fillRoundRect(x, y, w, h, 6, fill);
  td_canvas->drawRoundRect(x, y, w, h, 6, border);
  td_draw_centered_text(label, x, y + ((h - 8) / 2), w, 1, text);
}

void td_draw_usb_icon(int x, int y, uint16_t colour) {
  td_canvas->fillRect(x + 3, y, 7, 10, colour);
  td_canvas->fillRect(x + 4, y + 10, 5, 4, colour);
  td_canvas->drawPixel(x + 1, y + 3, colour);
  td_canvas->drawPixel(x + 11, y + 3, colour);
}

void td_draw_bt_icon(int x, int y, uint16_t colour) {
  td_canvas->drawLine(x + 5, y, x + 5, y + 13, colour);
  td_canvas->drawLine(x + 5, y, x + 9, y + 4, colour);
  td_canvas->drawLine(x + 9, y + 4, x + 0, y + 10, colour);
  td_canvas->drawLine(x + 0, y + 3, x + 9, y + 9, colour);
  td_canvas->drawLine(x + 9, y + 9, x + 5, y + 13, colour);
}

void td_draw_lora_icon(int x, int y, uint16_t colour) {
  td_canvas->fillCircle(x + 5, y + 7, 2, colour);
  td_canvas->drawCircle(x + 5, y + 7, 5, colour);
  td_canvas->drawCircle(x + 5, y + 7, 8, colour);
}

void td_draw_battery(int x, int y) {
  if (!pmu_ready || !battery_ready || !battery_installed) {
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_TEXT_DIM);
    td_canvas->setCursor(x, y + 1);
    td_canvas->print("PWR");
    return;
  }

  float bval = battery_percent;
  bool known = !battery_indeterminate;
  bool charging = known && (battery_state == BATTERY_STATE_CHARGING);
  bool charged = known && (battery_state == BATTERY_STATE_CHARGED);
  uint16_t fill_clr = TD_CLR_GREEN;

  if (charged) { fill_clr = TD_CLR_ACCENT; bval = 100.0f; }
  else if (charging) {
    fill_clr = TD_CLR_ACCENT;
    td_charge_tick += 3;
    if (td_charge_tick > 100) td_charge_tick = 0;
    bval = td_charge_tick;
  } else if (bval <= 20.0f) fill_clr = 0xF800;
  else if (bval <= 60.0f) fill_clr = 0xFBE0;

  int bw = 22; int bh = 10;
  td_canvas->drawRect(x, y, bw, bh, TD_CLR_TEXT_SECONDARY);
  td_canvas->fillRect(x + bw, y + 3, 2, 4, TD_CLR_TEXT_SECONDARY);
  int fill_w = (int)((bval / 100.0f) * (bw - 2));
  if (fill_w > 0) td_canvas->fillRect(x + 1, y + 1, fill_w, bh - 2, fill_clr);

  char bbuf[6];
  float pct = battery_percent;
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f) pct = 0.0f;
  snprintf(bbuf, sizeof(bbuf), "%d%%", (int)(pct + 0.5f));
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
  td_canvas->setCursor(x + 28, y + 1);
  td_canvas->print(bbuf);
}

void td_draw_statusbar() {
  td_canvas->fillRect(0, 0, TD_W, TD_BAR_H, TD_CLR_BG_STATUSBAR);

  td_draw_usb_icon(6, 5, (cable_state == CABLE_STATE_CONNECTED) ? TD_CLR_ACCENT : TD_CLR_TEXT_DIM);

  uint16_t bt_clr = TD_CLR_TEXT_DIM;
  if (bt_state == BT_STATE_ON) bt_clr = TD_CLR_ACCENT;
  else if (bt_state == BT_STATE_PAIRING) bt_clr = TD_CLR_WARN;
  else if (bt_state == BT_STATE_CONNECTED) bt_clr = TD_CLR_GREEN;
  td_draw_bt_icon(26, 5, bt_clr);

  td_draw_lora_icon(46, 5, radio_online ? TD_CLR_GREEN : TD_CLR_TEXT_DIM);

  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(118, 5);
  td_canvas->print("Ratspeak");

  td_draw_battery(264, 7);
  td_canvas->drawFastHLine(0, TD_BAR_H, TD_W, TD_CLR_BORDER);
}

void td_draw_pair_panel() {
  int cx = TD_WF_X + (TD_WF_W / 2);

  if (bt_state == BT_STATE_PAIRING) {
    uint32_t elapsed = millis() - bt_pairing_started;
    uint32_t remain_s = 0;
    if (elapsed < TD_PAIRING_WINDOW_MS) {
      remain_s = (TD_PAIRING_WINDOW_MS - elapsed + 999) / 1000;
    }
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_WARN);
    td_canvas->setCursor(cx - 23, TD_WF_Y + 56);
    td_canvas->print("PAIRING");
    char rbuf[4];
    snprintf(rbuf, sizeof(rbuf), "%lu", (unsigned long)remain_s);
    td_canvas->setTextSize(4);
    td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
    td_canvas->setCursor(remain_s >= 10 ? cx - 23 : cx - 11, TD_WF_Y + 76);
    td_canvas->print(rbuf);
    td_draw_button(TD_BTN_X, TD_CANCEL_BTN_Y, TD_BTN_W, TD_SETTINGS_BTN_H, "Cancel", TD_CLR_WARN, TD_CLR_WARN);
    return;
  }

  if (bt_state == BT_STATE_CONNECTED) {
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_GREEN);
    td_canvas->setCursor(cx - 39, TD_WF_Y + 80);
    td_canvas->print("BLE connected");
    td_canvas->setTextColor(TD_CLR_TEXT_DIM);
    td_canvas->setCursor(cx - 47, TD_WF_Y + 96);
    td_canvas->print("waiting for host");
    td_draw_button(TD_BTN_X, TD_SETTINGS_BTN_Y, TD_BTN_W, TD_SETTINGS_BTN_H, "Settings", TD_CLR_ACCENT, TD_CLR_ACCENT);
    return;
  }

  // Idle: paired devices can connect without opening pairing. The same panel
  // remains tappable to pair an additional or re-paired host.
  bool has_bond = bt_bond_count() > 0;
  td_canvas->fillRoundRect(TD_BTN_X, TD_PAIR_BTN_Y, TD_BTN_W, TD_PAIR_BTN_H, 8, TD_CLR_BG_PANEL);
  td_canvas->drawRoundRect(TD_BTN_X, TD_PAIR_BTN_Y, TD_BTN_W, TD_PAIR_BTN_H, 8, has_bond ? TD_CLR_GREEN : TD_CLR_ACCENT);
  td_canvas->setTextSize(1);
  if (has_bond) {
    td_canvas->setTextColor(TD_CLR_GREEN);
    td_canvas->setCursor(TD_BTN_X + 23, TD_PAIR_BTN_Y + 8);
    td_canvas->print("BLE ready");
    td_canvas->setTextColor(TD_CLR_TEXT_DIM);
    td_canvas->setCursor(TD_BTN_X + 19, TD_PAIR_BTN_Y + 23);
    td_canvas->print("tap to pair");
  } else {
    td_canvas->setTextColor(TD_CLR_ACCENT);
    td_canvas->setCursor(TD_BTN_X + 25, TD_PAIR_BTN_Y + 10);
    td_canvas->print("Pair via");
    td_canvas->setCursor(TD_BTN_X + 40, TD_PAIR_BTN_Y + 24);
    td_canvas->print("BLE");
  }
  td_draw_button(TD_BTN_X, TD_SETTINGS_BTN_Y, TD_BTN_W, TD_SETTINGS_BTN_H, "Settings", TD_CLR_ACCENT, TD_CLR_ACCENT);
}

void td_draw_waterfall() {
  td_canvas->fillRect(TD_WF_X, TD_WF_Y, TD_WF_W, TD_PANEL_H, TD_CLR_BG);
  td_canvas->drawFastVLine(TD_WF_X - 2, TD_WF_Y, TD_PANEL_H, TD_CLR_BORDER);

  if (!radio_online) {
    td_draw_pair_panel();
    return;
  }

  int rssi_val = current_rssi;
  if (rssi_val < WF_RSSI_MIN) rssi_val = WF_RSSI_MIN;
  if (rssi_val > WF_RSSI_MAX) rssi_val = WF_RSSI_MAX;
  int rssi_norm = ((rssi_val - WF_RSSI_MIN) * 8) / WF_RSSI_SPAN;
  if (rssi_norm > 7) rssi_norm = 7;
  if (rssi_norm < 0) rssi_norm = 0;

  if (display_tx) {
    for (uint8_t i = 0; i < WF_TX_SIZE; i++) {
      td_waterfall[td_waterfall_head] = -1;
      td_waterfall_head = (td_waterfall_head + 1) % TD_WF_SIZE;
    }
    display_tx = false;
  } else {
    td_waterfall[td_waterfall_head] = interference_detected ? 8 : rssi_norm;
    td_waterfall_head = (td_waterfall_head + 1) % TD_WF_SIZE;
  }

  for (int i = 0; i < TD_WF_SIZE; i++) {
    int wi = (td_waterfall_head + i) % TD_WF_SIZE;
    int ws = td_waterfall[wi];
    int ypos = TD_WF_Y + i;
    if (ws >= 0) {
      int bar_w = (ws * (TD_WF_W - 4)) / 8;
      if (bar_w > 0) td_canvas->drawFastHLine(TD_WF_X + 1, ypos, bar_w, td_wf_palette[ws]);
    } else {
      for (int tx = 0; tx < TD_WF_W - 4; tx += 2) {
        td_canvas->drawPixel(TD_WF_X + 1 + tx, ypos, 0xFFFF);
      }
    }
  }

  if (td_settings_button_visible()) {
    td_draw_button(TD_BTN_X, TD_ONLINE_SETTINGS_BTN_Y, TD_BTN_W, TD_SETTINGS_BTN_H, "Settings", TD_CLR_ACCENT, TD_CLR_ACCENT);
  }
}

void td_draw_idle_content() {
  int x = 8;
  int y = TD_CONTENT_Y + 4;

  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(x, y);
  if (bt_devname[0] != 0) td_canvas->print(bt_devname);
  else td_canvas->print("RNode");
  y += 22;

  char idbuf[8];
  snprintf(idbuf, sizeof(idbuf), "%02X%02X", (uint8_t)bt_dh[14], (uint8_t)bt_dh[15]);
  td_canvas->setTextSize(5);
  td_canvas->setTextColor(TD_CLR_ACCENT);
  td_canvas->setCursor(x, y);
  td_canvas->print(idbuf);
  y += 46;

  uint32_t freq_hz = (radio_online && LoRa != nullptr) ? LoRa->getFrequency() : lora_freq;
  char fbuf[20];
  snprintf(fbuf, sizeof(fbuf), "%.3f MHz", (float)freq_hz / 1000000.0f);
  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(x, y);
  td_canvas->print(fbuf);
  y += 20;

  uint32_t bw_hz = lora_bw;
  if (bw_hz == 0 && radio_online && LoRa != nullptr) bw_hz = LoRa->getSignalBandwidth();
  char bwstr[10];
  if (bw_hz >= 1000) snprintf(bwstr, sizeof(bwstr), "%luK", (unsigned long)(bw_hz / 1000));
  else snprintf(bwstr, sizeof(bwstr), "%lu", (unsigned long)bw_hz);
  char modbuf[36];
  int txp = (radio_online && LoRa != nullptr) ? (int)LoRa->getTxPower() : (int)lora_txp;
  snprintf(modbuf, sizeof(modbuf), "SF%d %s CR4/%d %+ddBm", lora_sf, bwstr, lora_cr, txp);
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
  td_canvas->setCursor(x, y);
  td_canvas->print(modbuf);
  y += 12;

  if (radio_online) {
    char brbuf[24];
    snprintf(brbuf, sizeof(brbuf), "%.2f Kbps", (float)lora_bitrate / 1000.0f);
    td_canvas->setCursor(x, y);
    td_canvas->print(brbuf);
  }
  y += 14;

  td_canvas->setCursor(x, y);
  if (radio_online) {
    td_canvas->setTextColor(TD_CLR_GREEN);
    td_canvas->print("Radio Online");
  } else {
    td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
    td_canvas->print("Ready to connect.");
  }
  y += 16;

  td_canvas->drawFastHLine(x, y, TD_LEFT_W - 16, TD_CLR_BORDER);
  y += 5;

  float at_st = airtime * 100.0f;
  float at_lt = longterm_airtime * 100.0f;
  float cl_st = total_channel_util * 100.0f;
  float cl_lt = longterm_channel_util * 100.0f;
  char abuf[40];
  td_canvas->setTextColor(td_load_color(at_st));
  snprintf(abuf, sizeof(abuf), "Air  ST %.1f%%  LT %.1f%%", at_st, at_lt);
  td_canvas->setCursor(x, y);
  td_canvas->print(abuf);
  y += 12;
  td_canvas->setTextColor(td_load_color(cl_st));
  snprintf(abuf, sizeof(abuf), "Chan ST %.1f%%  LT %.1f%%", cl_st, cl_lt);
  td_canvas->setCursor(x, y);
  td_canvas->print(abuf);
  y += 16;

  // last_rssi is -292 (sentinel) until a packet has actually been received
  char sigbuf[32];
  float rssi_pct = 0.0f;
  if (radio_online && last_rssi > -200) {
    signed char t_snr = (signed char)last_snr_raw;
    float snr = ((int)t_snr) * 0.25f;
    snprintf(sigbuf, sizeof(sigbuf), "RSSI %d   SNR %.1f", last_rssi, snr);
    rssi_pct = ((float)(last_rssi - S_RSSI_MIN) / S_RSSI_SPAN) * 100.0f;
  } else {
    snprintf(sigbuf, sizeof(sigbuf), "RSSI --   SNR --");
  }
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(x, y);
  td_canvas->print(sigbuf);
  y += 12;

  td_draw_gradient_bar(x, y, TD_LEFT_W - 16, 10, rssi_pct);
}

void td_draw_pairing_screen() {
  int x = 8;
  int y = TD_CONTENT_Y + 10;

  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_WARN);
  td_canvas->setCursor(x, y);
  td_canvas->print("BLE Pairing");
  y += 30;

  if (bt_ssp_pin != 0) {
    char pin_str[7];
    snprintf(pin_str, sizeof(pin_str), "%06d", (int)bt_ssp_pin);
    int dx = x;
    for (int i = 0; i < 6; i++) {
      td_canvas->fillRoundRect(dx, y, 28, 44, 5, TD_CLR_BG_PANEL);
      td_canvas->drawRoundRect(dx, y, 28, 44, 5, TD_CLR_ACCENT);
      td_canvas->setTextSize(4);
      td_canvas->setTextColor(TD_CLR_ACCENT);
      td_canvas->setCursor(dx + 4, y + 8);
      td_canvas->print(pin_str[i]);
      dx += 33;
    }
    y += 58;
  }
}

void td_draw_error_screen() {
  int x = 8;
  int y = TD_CONTENT_Y + 10;

  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_ERROR);
  td_canvas->setCursor(x, y);

  if (!device_firmware_ok()) {
    td_canvas->print("FW Corrupt");
    y += 26;
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
    td_canvas->setCursor(x, y);
    td_canvas->print("Reflash firmware to fix");
  } else if (!modem_installed) {
    td_canvas->print("No Radio");
    y += 26;
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
    td_canvas->setCursor(x, y);
    td_canvas->print("Check radio module");
  } else {
    td_canvas->print("Config Error");
    y += 26;
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
    td_canvas->setCursor(x, y);
    td_canvas->print("Run rnodeconf");
  }
}

void td_draw_settings_screen() {
  td_canvas->fillRect(0, TD_BAR_H + 1, TD_W, TD_FOOT_Y - TD_BAR_H - 1, TD_CLR_BG);

  int x = 12;
  int y = TD_CONTENT_Y + 6;
  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(x, y);
  td_canvas->print("Settings");
  td_draw_button(TD_SETTINGS_BACK_X, TD_SETTINGS_BACK_Y, TD_SETTINGS_BACK_W, TD_SETTINGS_BACK_H, "Back", TD_CLR_BORDER, TD_CLR_TEXT_SECONDARY);

  y += 34;
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
  td_canvas->setCursor(x, y);
  td_canvas->print("Brightness");

  char pctbuf[8];
  snprintf(pctbuf, sizeof(pctbuf), "%u%%", (unsigned int)td_settings_brightness_pct);
  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_ACCENT);
  td_canvas->setCursor(214, y - 5);
  td_canvas->print(pctbuf);

  td_draw_button(TD_SETTINGS_MINUS_X, TD_SETTINGS_MINUS_Y, TD_SETTINGS_STEP_W, TD_SETTINGS_STEP_H, "-", TD_CLR_BORDER, TD_CLR_TEXT_PRIMARY);
  td_draw_button(TD_SETTINGS_PLUS_X, TD_SETTINGS_PLUS_Y, TD_SETTINGS_STEP_W, TD_SETTINGS_STEP_H, "+", TD_CLR_BORDER, TD_CLR_TEXT_PRIMARY);

  td_canvas->drawRect(TD_SETTINGS_BAR_X, TD_SETTINGS_BAR_Y, TD_SETTINGS_BAR_W, TD_SETTINGS_BAR_H, TD_CLR_BORDER);
  int usable = TD_SETTINGS_BAR_W - 2;
  int fill = ((td_settings_brightness_pct - TD_BRIGHTNESS_MIN_PCT) * usable) / (100 - TD_BRIGHTNESS_MIN_PCT);
  if (fill > 0) td_canvas->fillRect(TD_SETTINGS_BAR_X + 1, TD_SETTINGS_BAR_Y + 1, fill, TD_SETTINGS_BAR_H - 2, TD_CLR_ACCENT);
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_DIM);
  td_canvas->setCursor(TD_SETTINGS_BAR_X, TD_SETTINGS_BAR_Y + 14);
  td_canvas->print("min 5%");

  td_draw_button(TD_SETTINGS_ABOUT_X, TD_SETTINGS_ABOUT_Y, TD_SETTINGS_ABOUT_W, TD_SETTINGS_ABOUT_H, "About", TD_CLR_GREEN, TD_CLR_GREEN);
  td_draw_button(TD_SETTINGS_SLEEP_X, TD_SETTINGS_SLEEP_Y, TD_SETTINGS_SLEEP_W, TD_SETTINGS_SLEEP_H, "Sleep Screen", TD_CLR_ACCENT, TD_CLR_ACCENT);

  bool confirm_forget = (int32_t)(td_settings_forget_until - millis()) > 0;
  td_draw_button(
    TD_SETTINGS_FORGET_X,
    TD_SETTINGS_FORGET_Y,
    TD_SETTINGS_FORGET_W,
    TD_SETTINGS_FORGET_H,
    confirm_forget ? "Confirm Forget BLE" : "Forget BLE Bonds",
    confirm_forget ? TD_CLR_WARN : TD_CLR_BORDER,
    confirm_forget ? TD_CLR_WARN : TD_CLR_TEXT_SECONDARY,
    confirm_forget
  );

  if (td_settings_status_msg && (int32_t)(td_settings_status_until - millis()) > 0) {
    td_draw_centered_text(td_settings_status_msg, 10, TD_FOOT_Y - 17, TD_W - 20, 1, TD_CLR_TEXT_DIM);
  } else {
    td_settings_status_msg = nullptr;
  }
}

void td_draw_about_screen() {
  td_canvas->fillRect(0, TD_BAR_H + 1, TD_W, TD_FOOT_Y - TD_BAR_H - 1, TD_CLR_BG);

  int px = 34;
  int py = 44;
  int pw = 252;
  int ph = 168;
  td_canvas->fillRoundRect(px, py, pw, ph, 8, TD_CLR_BG_PANEL);
  td_canvas->drawRoundRect(px, py, pw, ph, 8, TD_CLR_ACCENT);

  td_canvas->setTextSize(2);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(px + 20, py + 16);
  td_canvas->print("About RNode");

  char line[36];
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);

  snprintf(line, sizeof(line), "rsDeck v%d.%d.%d", RSDECK_VERSION_MAJOR, RSDECK_VERSION_MINOR, RSDECK_VERSION_PATCH);
  td_canvas->setCursor(px + 20, py + 54);
  td_canvas->print(line);

  snprintf(line, sizeof(line), "RNode v%d.%02d", MAJ_VERS, MIN_VERS);
  td_canvas->setCursor(px + 20, py + 72);
  td_canvas->print(line);

  uint32_t total_s = millis() / 1000;
  uint32_t days = total_s / 86400;
  uint32_t hours = (total_s / 3600) % 24;
  uint32_t mins = (total_s / 60) % 60;
  uint32_t secs = total_s % 60;
  if (days > 0) {
    snprintf(line, sizeof(line), "Uptime %lud %02lu:%02lu:%02lu",
      (unsigned long)days, (unsigned long)hours, (unsigned long)mins, (unsigned long)secs);
  } else {
    snprintf(line, sizeof(line), "Uptime %02lu:%02lu:%02lu",
      (unsigned long)hours, (unsigned long)mins, (unsigned long)secs);
  }
  td_canvas->setCursor(px + 20, py + 90);
  td_canvas->print(line);

  int bonds = bt_bond_count();
  snprintf(line, sizeof(line), "BLE bonds %d", bonds);
  td_canvas->setCursor(px + 20, py + 108);
  td_canvas->print(line);

  td_draw_button(TD_ABOUT_CLOSE_X, TD_ABOUT_CLOSE_Y, TD_ABOUT_CLOSE_W, TD_ABOUT_CLOSE_H, "Close", TD_CLR_ACCENT, TD_CLR_ACCENT);
}

void td_draw_external_framebuffer() {
  int scale = 3;
  int ox = (TD_LEFT_W - 64 * scale) / 2;
  int oy = TD_CONTENT_Y + 2;
  td_canvas->drawRect(ox - 2, oy - 2, 64 * scale + 4, 64 * scale + 4, TD_CLR_BORDER);
  for (int row = 0; row < 64; row++) {
    for (int col = 0; col < 64; col++) {
      int index = row * 8 + (col / 8);
      uint8_t bitmask = 1 << (7 - (col % 8));
      if (fb[index] & bitmask) {
        td_canvas->fillRect(ox + col * scale, oy + row * scale, scale, scale, 0xFFFF);
      }
    }
  }
}

void td_draw_boot_screen() {
  td_canvas->fillScreen(TD_CLR_BG);
  td_canvas->setTextSize(4);
  td_canvas->setTextColor(TD_CLR_ACCENT);
  td_canvas->setCursor(100, 80);
  td_canvas->print("RNode");
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
  td_canvas->setCursor(122, 122);
  td_canvas->print("rsDeck  T-Deck");

  int bar_w = 200;
  int bar_x = (TD_W - bar_w) / 2;
  int bar_y = 150;
  td_canvas->drawRect(bar_x, bar_y, bar_w, 8, TD_CLR_BORDER);
  int pulse = (millis() / 15) % bar_w;
  int pw = 50;
  int p_start = pulse - pw; if (p_start < 0) p_start = 0;
  int p_end = pulse; if (p_end > bar_w - 2) p_end = bar_w - 2;
  if (p_end > p_start) {
    td_canvas->fillRect(bar_x + 1 + p_start, bar_y + 1, p_end - p_start, 6, TD_CLR_ACCENT);
  }
}

void td_draw_footer() {
  td_canvas->drawFastHLine(0, TD_FOOT_Y, TD_W, TD_CLR_BORDER);
  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_DIM);
  td_canvas->setCursor(8, TD_FOOT_Y + 4);
  char vbuf[20];
  snprintf(vbuf, sizeof(vbuf), "Firmware v%d.%02d", MAJ_VERS, MIN_VERS);
  td_canvas->print(vbuf);
}

void td_render_frame() {
  if (!td_ui_ready) td_ui_init();
  if (!td_canvas || !td_canvas->getBuffer()) return;

  if (!device_init_done) {
    td_draw_boot_screen();
    return;
  }

  if (firmware_update_mode) {
    td_canvas->fillScreen(TD_CLR_BG);
    td_canvas->setTextSize(2);
    td_canvas->setTextColor(TD_CLR_WARN);
    td_canvas->setCursor(40, 100);
    td_canvas->print("Firmware Update");
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_ERROR);
    td_canvas->setCursor(100, 130);
    td_canvas->print("Do not power off");
    return;
  }

  td_canvas->fillScreen(TD_CLR_BG);
  td_draw_statusbar();
  td_draw_footer();

  if (td_view == TD_VIEW_SETTINGS) {
    td_draw_settings_screen();
    return;
  }

  if (td_view == TD_VIEW_ABOUT) {
    td_draw_about_screen();
    return;
  }

  td_draw_waterfall();

  if (disp_ext_fb && bt_ssp_pin == 0) {
    td_draw_external_framebuffer();
  } else if (!hw_ready || radio_error || !device_firmware_ok()) {
    td_draw_error_screen();
  } else if (bt_state == BT_STATE_PAIRING && bt_ssp_pin != 0) {
    td_draw_pairing_screen();
  } else {
    td_draw_idle_content();
  }
}

#endif // TDECK_UI_H
