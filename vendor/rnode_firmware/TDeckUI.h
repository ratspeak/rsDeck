// Full-screen RNode UI for the LilyGo T-Deck (rsDeck dual-boot build).
// Renders into a PSRAM-backed 320x240 canvas pushed to the ST7789, replacing
// the generic 64x128 OLED layout. Included by Display.h for BOARD_TDECK only.
// Touch (GT911) toggles the display for battery savings; buttons and BT
// events wake it via the standard display_unblank() path.

#ifndef TDECK_UI_H
#define TDECK_UI_H

void display_unblank();

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
#define TD_WF_H 192
#define TD_WF_SIZE TD_WF_H

static const uint16_t td_wf_palette[9] = {
  0x0000, 0x0011, 0x001F, 0x07FF,
  0x07E0, 0xFFE0, 0xFBE0, 0xF800, 0xFFFF
};

GFXcanvas16* td_canvas = nullptr;
bool td_ui_ready = false;
int td_waterfall[TD_WF_SIZE];
int td_waterfall_head = 0;
uint8_t td_charge_tick = 0;

// --- GT911 touch (poll-only, tap toggles display power) ---
#define TD_TOUCH_SDA 18
#define TD_TOUCH_SCL 8
uint8_t td_touch_addr = 0;
bool td_touch_down = false;
uint32_t td_last_touch_poll = 0;

void td_touch_init() {
  Wire.begin(TD_TOUCH_SDA, TD_TOUCH_SCL, 400000);
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) { td_touch_addr = 0x5D; return; }
  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) { td_touch_addr = 0x14; }
}

bool td_touch_read_down() {
  if (td_touch_addr == 0) return false;
  Wire.beginTransmission(td_touch_addr);
  Wire.write(0x81); Wire.write(0x4E);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(td_touch_addr, (uint8_t)1);
  if (!Wire.available()) return false;
  uint8_t status = Wire.read();
  if (!(status & 0x80)) return false;
  // Clear the buffer-ready flag so the controller keeps updating
  Wire.beginTransmission(td_touch_addr);
  Wire.write(0x81); Wire.write(0x4E); Wire.write(0x00);
  Wire.endTransmission();
  return (status & 0x0F) > 0;
}

void td_poll_touch() {
  if (millis() - td_last_touch_poll < 50) return;
  td_last_touch_poll = millis();

  bool down = td_touch_read_down();
  if (down && !td_touch_down) {
    td_touch_down = true;
  } else if (!down && td_touch_down) {
    td_touch_down = false;
    // Tap released: toggle display power
    if (display_blanked) {
      display_unblank();
    } else {
      last_unblank_event = millis() - display_blanking_timeout - 1;
    }
    last_disp_update = 0;
  }
}

void td_ui_init() {
  if (td_ui_ready) return;
  display.setRotation(1);
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
  td_canvas->setCursor(106, 5);
  td_canvas->print("rsDeck RNode");

  td_draw_battery(264, 7);
  td_canvas->drawFastHLine(0, TD_BAR_H, TD_W, TD_CLR_BORDER);
}

void td_draw_waterfall() {
  td_canvas->fillRect(TD_WF_X, TD_WF_Y, TD_WF_W, TD_WF_H + 2, TD_CLR_BG);
  td_canvas->drawFastVLine(TD_WF_X - 2, TD_WF_Y, TD_WF_H, TD_CLR_BORDER);

  if (!radio_online) {
    td_canvas->setTextSize(1);
    td_canvas->setTextColor(TD_CLR_TEXT_DIM);
    td_canvas->setCursor(TD_WF_X + 18, TD_WF_Y + TD_WF_H / 2);
    td_canvas->print("radio off");
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

  char brbuf[24];
  snprintf(brbuf, sizeof(brbuf), "%.2f Kbps", (float)lora_bitrate / 1000.0f);
  td_canvas->setCursor(x, y);
  td_canvas->print(brbuf);
  y += 14;

  td_canvas->setCursor(x, y);
  if (radio_online) {
    td_canvas->setTextColor(TD_CLR_GREEN);
    td_canvas->print("Radio Online");
  } else if (!device_signatures_ok()) {
    td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
    td_canvas->print("Provisioned, host config pending");
  } else {
    td_canvas->setTextColor(TD_CLR_TEXT_DIM);
    td_canvas->print("Waiting for host");
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

  signed char t_snr = (signed char)last_snr_raw;
  float snr = ((int)t_snr) * 0.25f;
  char sigbuf[32];
  snprintf(sigbuf, sizeof(sigbuf), "RSSI %d   SNR %.1f", last_rssi, snr);
  td_canvas->setTextColor(TD_CLR_TEXT_PRIMARY);
  td_canvas->setCursor(x, y);
  td_canvas->print(sigbuf);
  y += 12;

  float rssi_pct = ((float)(last_rssi - S_RSSI_MIN) / S_RSSI_SPAN) * 100.0f;
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

  td_canvas->setTextSize(1);
  td_canvas->setTextColor(TD_CLR_TEXT_SECONDARY);
  td_canvas->setCursor(x, y);
  td_canvas->print("Confirm this code on the connecting device.");
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
  td_canvas->setCursor(170, TD_FOOT_Y + 4);
  td_canvas->print("click: pair   touch: screen");
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
  td_draw_waterfall();
  td_draw_footer();

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
