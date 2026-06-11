// Copyright (C) 2024, Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Graphics.h"
#include <Adafruit_GFX.h>

#if BOARD_MODEL != BOARD_TECHO
  #if BOARD_MODEL == BOARD_CARDPUTER_ADV
    #include <M5Unified.h>
    #include <M5GFX.h>
    #include <M5Cardputer.h>
    #define SSD1306_WHITE 0xFFFF
    #define SSD1306_BLACK 0x0000

    #define CLR_BG_STATUSBAR  0x18E3
    #define CLR_BG_PANEL      0x0861
    #define CLR_BORDER        0x4208
    #define CLR_TEXT_PRIMARY  0xFFFF
    #define CLR_TEXT_SECONDARY 0xB596
    #define CLR_TEXT_ACCENT   0x07FF
    #define CLR_TEXT_DIM      0x6B4D
    #define CLR_TEXT_WARN     0xFBE0
    #define CLR_TEXT_ERROR    0xF800
    #define CLR_CABLE_OFF     0x4208
    #define CLR_CABLE_ON      0x07E0
    #define CLR_BT_OFF        0x4208
    #define CLR_BT_ON         0x001F
    #define CLR_BT_PAIR       0xFBE0
    #define CLR_BT_CONN       0x07FF
    #define CLR_LORA_OFF      0x4208
    #define CLR_LORA_ON       0x07E0
    #define CLR_BAT_FULL      0x07E0
    #define CLR_BAT_MED       0xFBE0
    #define CLR_BAT_LOW       0xF800
    #define CLR_BAT_CHARGE    0x07FF
    #define CLR_BAT_OUTLINE   0x8410

    #define CP_WF_X       0
    #define CP_WF_Y       21
    #define CP_WF_W       22
    #define CP_WF_H       114
    #define CP_WF_BAR_W   20
    #define CP_WF_SIZE    114

    #define CP_CONTENT_X  23
    #define CP_CONTENT_Y  21
    #define CP_CONTENT_W  217
    #define CP_CONTENT_H  114

    static const uint16_t wf_palette[9] = {
      0x0000, 0x0011, 0x001F, 0x07FF,
      0x07E0, 0xFFE0, 0xFBE0, 0xF800, 0xFFFF
    };
  #elif BOARD_MODEL == BOARD_TDECK
    #include <Adafruit_ST7789.h>
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    #include "ST7789.h"
    #define COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
  #elif BOARD_MODEL == BOARD_TBEAM_S_V1
    #include <Adafruit_SH110X.h>
  #else
    #include <Wire.h>
    #include <Adafruit_SSD1306.h>
  #endif

#else
  void (*display_callback)();
  void display_add_callback(void (*callback)()) { display_callback = callback; }
  void busyCallback(const void* p) { display_callback(); }
  #define SSD1306_BLACK GxEPD_BLACK
  #define SSD1306_WHITE GxEPD_WHITE
  #include <GxEPD2_BW.h>
  #include <SPI.h>
#endif

#include "Fonts/Org_01.h"
#define DISP_W 128
#define DISP_H 64

#if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_TBEAM
  #define DISP_RST 13
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#elif BOARD_MODEL == BOARD_HELTEC32_V2 || BOARD_MODEL == BOARD_LORA32_V1_0
  #define DISP_RST 16
  #define DISP_ADDR 0x3C
  #define SCL_OLED 15
  #define SDA_OLED 4
#elif BOARD_MODEL == BOARD_HELTEC32_V3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_HELTEC32_V4
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_RAK4631
  // RAK1921/SSD1306
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 14
  #define SDA_OLED 13
#elif BOARD_MODEL == BOARD_RNODE_NG_21
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_T3S3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 17
  #define SDA_OLED 18
#elif BOARD_MODEL == BOARD_TECHO
  SPIClass displaySPI = SPIClass(NRF_SPIM0, pin_disp_miso, pin_disp_sck, pin_disp_mosi);
  #define DISP_W 128
  #define DISP_H 64
  #define DISP_ADDR -1
#elif BOARD_MODEL == BOARD_TBEAM_S_V1
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
  #define DISP_CUSTOM_ADDR false
#elif BOARD_MODEL == BOARD_XIAO_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 6
  #define SDA_OLED 5
  #define DISP_CUSTOM_ADDR true
#elif BOARD_MODEL == BOARD_CARDPUTER_ADV
  #undef DISP_W
  #undef DISP_H
  #define DISP_W 240
  #define DISP_H 135
  #define DISP_ADDR -1
  #define DISP_CUSTOM_ADDR false
#else
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#endif

#define SMALL_FONT &Org_01

#if BOARD_MODEL == BOARD_TDECK
  Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS, DISPLAY_DC, -1);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
#elif BOARD_MODEL == BOARD_HELTEC_T114
  ST7789Spi display(&SPI1, DISPLAY_RST, DISPLAY_DC, DISPLAY_CS);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
#elif BOARD_MODEL == BOARD_TBEAM_S_V1
  Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
  #define SSD1306_WHITE SH110X_WHITE
  #define SSD1306_BLACK SH110X_BLACK
#elif BOARD_MODEL == BOARD_CARDPUTER_ADV
  M5Canvas m5canvas(&M5Cardputer.Display);
#elif BOARD_MODEL == BOARD_TECHO
  GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(pin_disp_cs, pin_disp_dc, pin_disp_reset, pin_disp_busy));
  uint32_t last_epd_refresh = 0;
  uint32_t last_epd_full_refresh = 0;
  #define REFRESH_PERIOD 300000
#else
  Adafruit_SSD1306 display(DISP_W, DISP_H, &Wire, DISP_RST);
#endif

#if BOARD_MODEL == BOARD_CARDPUTER_ADV
  float disp_target_fps = 3;
#else
  float disp_target_fps = 7;
#endif
float epd_update_fps  = 0.5;

#define DISP_MODE_UNKNOWN   0x00
#define DISP_MODE_LANDSCAPE 0x01
#define DISP_MODE_PORTRAIT  0x02
#define DISP_PIN_SIZE   6
#if BOARD_MODEL == BOARD_CARDPUTER_ADV
  #define DISPLAY_BLANKING_TIMEOUT 30*1000
  #define CARDPUTER_ADV_DISPLAY_INTENSITY_DEFAULT 96
#else
  #define DISPLAY_BLANKING_TIMEOUT 15*1000
#endif
uint8_t disp_mode = DISP_MODE_UNKNOWN;
uint8_t disp_ext_fb = false;
unsigned char fb[512];
uint32_t last_disp_update = 0;
uint32_t last_unblank_event = 0;
uint32_t display_blanking_timeout = DISPLAY_BLANKING_TIMEOUT;
uint8_t display_unblank_intensity = display_intensity;
bool display_blanked = false;
bool display_blank_frame_drawn = false;
bool display_tx = false;
bool recondition_display = false;
int disp_update_interval = 1000/disp_target_fps;
int epd_update_interval = 1000/disp_target_fps;
uint32_t last_page_flip = 0;
int page_interval = 4000;
bool device_signatures_ok();
bool device_firmware_ok();

#define WATERFALL_SIZE 46
int waterfall[WATERFALL_SIZE];
int waterfall_meta[WATERFALL_SIZE];
int waterfall_head = 0;

int p_ad_x = 0;
int p_ad_y = 0;
int p_as_x = 0;
int p_as_y = 0;

#define START_PAGE 0
const uint8_t pages = 3;
uint8_t disp_page = START_PAGE;

#if BOARD_MODEL != BOARD_CARDPUTER_ADV
  GFXcanvas1 stat_area(64, 64);
  GFXcanvas1 disp_area(64, 64);
#else
  int cp_waterfall[CP_WF_SIZE] = {0};
  int cp_waterfall_head = 0;
  uint8_t cp_charge_tick = 0;
  uint32_t cp_pairing_tip_until = 0;
  uint32_t cp_ble_notice_until = 0;
  bool cp_ble_notice_enabled = false;
#endif

static const uint8_t one_counts[256] = {
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,
  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  2,  3,
  2,  2,  2,  2,  2,  2,  2,  2,  1,  2,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,
  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0
};

void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour);

void update_area_positions() {
  #if BOARD_MODEL == BOARD_HELTEC_T114
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 16;
      p_ad_y = 64;
      p_as_x = 16;
      p_as_y = p_ad_y+126;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 96;
      p_as_x = 126;
      p_as_y = p_ad_y;
    }
  #elif BOARD_MODEL == BOARD_TECHO
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 61;
      p_ad_y = 36;
      p_as_x = 64;
      p_as_y = 64+36;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 0;
      p_as_x = 64;
      p_as_y = 0;
    }
  #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
    p_ad_x = 0;
    p_ad_y = 0;
    p_as_x = 64;
    p_as_y = 0;
  #else
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 0 * DISPLAY_SCALE;
      p_as_y = 64 * DISPLAY_SCALE;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 64 * DISPLAY_SCALE;
      p_as_y = 0 * DISPLAY_SCALE;
    }
  #endif
}

uint8_t display_contrast = 0x00;
#if BOARD_MODEL == BOARD_TBEAM_S_V1
  void set_contrast(Adafruit_SH1106G *display, uint8_t value) {
  }
#elif BOARD_MODEL == BOARD_HELTEC_T114
  void set_contrast(ST7789Spi *display, uint8_t value) { }
#elif BOARD_MODEL == BOARD_TECHO
  void set_contrast(void *display, uint8_t value) {
    if (value == 0) { analogWrite(pin_backlight, 0); }
    else            { analogWrite(pin_backlight, value); }
  }
#elif BOARD_MODEL == BOARD_TDECK
  void set_contrast(Adafruit_ST7789 *display, uint8_t value) {
    static uint8_t level = 0;
    static uint8_t steps = 16;
    if (value > 15) value = 15;
    if (value == 0) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        delay(3);
        level = 0;
        return;
    }
    if (level == 0) {
        digitalWrite(DISPLAY_BL_PIN, 1);
        level = steps;
        delayMicroseconds(30);
    }
    int from = steps - level;
    int to = steps - value;
    int num = (steps + to - from) % steps;
    for (int i = 0; i < num; i++) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        digitalWrite(DISPLAY_BL_PIN, 1);
    }
    level = value;
  }
#elif BOARD_MODEL == BOARD_CARDPUTER_ADV
  void set_contrast(M5Canvas *display, uint8_t value) {
    M5Cardputer.Display.setBrightness(value);
  }
#else
  void set_contrast(Adafruit_SSD1306 *display, uint8_t contrast) {
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(contrast);
  }
#endif

bool display_init() {
  #if HAS_DISPLAY
    #if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
    #elif BOARD_MODEL == BOARD_T3S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V2
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V3
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V4
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_LORA32_V1_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC_T114
      pinMode(PIN_T114_TFT_EN, OUTPUT);
      digitalWrite(PIN_T114_TFT_EN, LOW);
    #elif BOARD_MODEL == BOARD_TECHO
      display.init(0, true, 10, false, displaySPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
      display.setPartialWindow(0, 0, DISP_W, DISP_H);
      display.epd2.setBusyCallback(busyCallback);
      #if HAS_BACKLIGHT
        pinMode(pin_backlight, OUTPUT);
        analogWrite(pin_backlight, 0);
      #endif
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_XIAO_S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
      {
        auto cfg = M5.config();
        cfg.internal_spk = false;
        cfg.internal_mic = false;
        M5Cardputer.begin(cfg, true);
      }
    #endif

    #if HAS_EEPROM
      uint8_t display_rotation = EEPROM.read(eeprom_addr(ADDR_CONF_DROT));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t display_rotation = eeprom_read(eeprom_addr(ADDR_CONF_DROT));
    #endif
    if (display_rotation < 0 or display_rotation > 3) display_rotation = 0xFF;

    #if DISP_CUSTOM_ADDR == true
      #if HAS_EEPROM
        uint8_t display_address = EEPROM.read(eeprom_addr(ADDR_CONF_DADR));
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t display_address = eeprom_read(eeprom_addr(ADDR_CONF_DADR));
      #endif
      if (display_address == 0xFF) display_address = DISP_ADDR;
    #else
      uint8_t display_address = DISP_ADDR;
    #endif

      #if BOARD_MODEL == BOARD_CARDPUTER_ADV
        display_blanking_enabled = true;
        display_blanking_timeout = DISPLAY_BLANKING_TIMEOUT;
      #endif

      #if HAS_EEPROM
        if (EEPROM.read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
          uint8_t db_timeout = EEPROM.read(eeprom_addr(ADDR_CONF_DBLK));
          if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #elif MCU_VARIANT == MCU_NRF52
      if (eeprom_read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
        uint8_t db_timeout = eeprom_read(eeprom_addr(ADDR_CONF_DBLK));
        if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #endif
    
    #if BOARD_MODEL == BOARD_TECHO
    // Don't check if display is actually connected
    if(false) {
    #elif BOARD_MODEL == BOARD_TDECK
    display.init(240, 320);
    display.setSPISpeed(80e6);
    if (false) {
    #elif BOARD_MODEL == BOARD_HELTEC_T114
    display.init();
    // set white as default pixel colour for Heltec T114
    display.setRGB(COLOR565(0xFF, 0xFF, 0xFF));
    if (false) {
    #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
    if (false) {
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1
    if (!display.begin(display_address, true)) {
    #else
    if (!display.begin(SSD1306_SWITCHCAPVCC, display_address)) {
    #endif
      return false;
    } else {
      #if BOARD_MODEL == BOARD_CARDPUTER_ADV
        set_contrast(&m5canvas, display_contrast);
      #else
        set_contrast(&display, display_contrast);
      #endif
      if (display_rotation != 0xFF) {
        #if BOARD_MODEL == BOARD_CARDPUTER_ADV
          M5Cardputer.Display.setRotation(display_rotation);
          if (M5Cardputer.Display.width() >= M5Cardputer.Display.height()) {
            disp_mode = DISP_MODE_LANDSCAPE;
          } else {
            disp_mode = DISP_MODE_PORTRAIT;
          }
        #else
          if (display_rotation == 0 || display_rotation == 2) {
            disp_mode = DISP_MODE_LANDSCAPE;
          } else {
            disp_mode = DISP_MODE_PORTRAIT;
          }
          display.setRotation(display_rotation);
        #endif
      } else {
        #if BOARD_MODEL == BOARD_RNODE_NG_20
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_RNODE_NG_21
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V1_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_1
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM_S_V1
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V2
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V3
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V4
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC_T114
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_RAK4631
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TDECK
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_TECHO
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
          disp_mode = DISP_MODE_LANDSCAPE;
          M5Cardputer.Display.setRotation(1);
        #else
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #endif
      }

      update_area_positions();

      for (int i = 0; i < WATERFALL_SIZE; i++) { waterfall[i] = 0; }

      last_page_flip = millis();

      #if BOARD_MODEL == BOARD_CARDPUTER_ADV
        M5Cardputer.Display.fillScreen(SSD1306_BLACK);
        m5canvas.setColorDepth(8);
        if (!m5canvas.createSprite(DISP_W, DISP_H)) {
          return false;
        }
        m5canvas.setPaletteColor(0, SSD1306_BLACK);
        m5canvas.setPaletteColor(1, CLR_BG_STATUSBAR);
        m5canvas.setPaletteColor(2, CLR_BG_PANEL);
        m5canvas.setPaletteColor(3, CLR_BORDER);
        m5canvas.setPaletteColor(4, CLR_TEXT_PRIMARY);
        m5canvas.setPaletteColor(5, CLR_TEXT_SECONDARY);
        m5canvas.setPaletteColor(6, CLR_TEXT_ACCENT);
        m5canvas.setPaletteColor(7, CLR_TEXT_WARN);
        m5canvas.setPaletteColor(8, CLR_TEXT_ERROR);
        m5canvas.setPaletteColor(9, CLR_LORA_ON);
        m5canvas.setPaletteColor(10, CLR_CABLE_ON);
        m5canvas.setTextSize(1);
        m5canvas.fillSprite(SSD1306_BLACK);
        m5canvas.pushSprite(0, 0);
      #else
        stat_area.cp437(true);
        disp_area.cp437(true);
      #endif

      #if BOARD_MODEL != BOARD_HELTEC_T114 && BOARD_MODEL != BOARD_CARDPUTER_ADV
      display.cp437(true);
      #endif

      #if HAS_EEPROM
        display_intensity = EEPROM.read(eeprom_addr(ADDR_CONF_DINT));
      #elif MCU_VARIANT == MCU_NRF52
        display_intensity = eeprom_read(eeprom_addr(ADDR_CONF_DINT));
      #endif
      #if BOARD_MODEL == BOARD_CARDPUTER_ADV
        if (display_intensity == 0xFF) {
          display_intensity = CARDPUTER_ADV_DISPLAY_INTENSITY_DEFAULT;
        }
      #endif
      display_unblank_intensity = display_intensity;

      #if BOARD_MODEL == BOARD_CARDPUTER_ADV
        set_contrast(&m5canvas, display_intensity);
      #endif

      #if BOARD_MODEL == BOARD_TECHO
        #if HAS_BACKLIGHT
          if (display_intensity == 0) { analogWrite(pin_backlight, 0); }
          else                        { analogWrite(pin_backlight, display_intensity); }
        #endif
      #endif

      #if BOARD_MODEL == BOARD_TDECK
        display.fillScreen(SSD1306_BLACK);
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        // Enable backlight led (display is always black without this)
        fillRect(p_ad_x, p_ad_y, 128, 128, SSD1306_BLACK);
        fillRect(p_as_x, p_as_y, 128, 128, SSD1306_BLACK);
        pinMode(PIN_T114_TFT_BLGT, OUTPUT);
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #endif

      return true;
    }
  #else
    return false;
  #endif
}

#if BOARD_MODEL != BOARD_CARDPUTER_ADV
// Draws a line on the screen
void drawLine(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.drawLine(x, y, width, height);
  #else
  display.drawLine(x, y, width, height, colour);
  #endif
}

// Draws a filled rectangle on the screen
void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.fillRect(x, y, width, height);
  #else
  display.fillRect(x, y, width, height, colour);
  #endif
}

// Draws a bitmap to the display and auto scales it based on the boards configured DISPLAY_SCALE
void drawBitmap(int16_t startX, int16_t startY, const uint8_t* bitmap, int16_t bitmapWidth, int16_t bitmapHeight, uint16_t foregroundColour, uint16_t backgroundColour) {
  #if DISPLAY_SCALE == 1
    display.drawBitmap(startX, startY, bitmap, bitmapWidth, bitmapHeight, foregroundColour, backgroundColour);
  #else
    for(int16_t row = 0; row < bitmapHeight; row++){
        for(int16_t col = 0; col < bitmapWidth; col++){

            // determine index and bitmask
            int16_t index = row * ((bitmapWidth + 7) / 8) + (col / 8);
            uint8_t bitmask = 1 << (7 - (col % 8));

            // check if the current pixel is set in the bitmap
            if(bitmap[index] & bitmask){
                // draw a scaled rectangle for the foreground pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, foregroundColour);
            } else {
                // draw a scaled rectangle for the background pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, backgroundColour);
            }

        }
    }
  #endif
}

extern uint8_t wifi_mode;
extern bool wifi_is_connected();
extern bool wifi_host_is_connected();
void draw_cable_icon(int px, int py) {
  #if HAS_WIFI
    if (wifi_mode == WR_WIFI_OFF) {
      if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    } else {
      if (wifi_mode == WR_WIFI_STA) {
        if (wifi_is_connected()) {
          stat_area.drawBitmap(px, py, bm_wifi+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
          if (!wifi_host_is_connected()) { stat_area.fillRect(px+5, py+12, 6, 3, SSD1306_BLACK); }
        } else { stat_area.drawBitmap(px, py, bm_wifi+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else if (wifi_mode == WR_WIFI_AP) {
        if (wifi_host_is_connected()) { stat_area.drawBitmap(px, py, bm_wifi+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else                          { stat_area.drawBitmap(px, py, bm_wifi+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else {
        if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      }
    }

  #else
  if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  #endif
}

void draw_bt_icon(int px, int py) {
  if (bt_state == BT_STATE_OFF) {
    stat_area.drawBitmap(px, py, bm_bt+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_ON) {
    stat_area.drawBitmap(px, py, bm_bt+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_PAIRING) {
    stat_area.drawBitmap(px, py, bm_bt+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_CONNECTED) {
    stat_area.drawBitmap(px, py, bm_bt+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_bt+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_lora_icon(int px, int py) {
  if (radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_mw_icon(int px, int py) {
  if (mw_radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

uint8_t charge_tick = 0;
void draw_battery_bars(int px, int py) {
  if (pmu_ready) {
    if (battery_ready) {
      if (battery_installed) {
        float battery_value = battery_percent;

        // Disable charging state display for now, since
        // boards without dedicated PMU are completely
        // unreliable for determining actual charging state.
        bool disable_charge_status = false;
        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING) {
          disable_charge_status = true;
        }
        
        if (battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          float battery_prog = battery_percent;
          if (battery_prog > 85) { battery_prog = 84; }
          if (charge_tick < battery_prog ) { charge_tick = battery_prog; }
          battery_value = charge_tick;
          charge_tick += 3;
          if (charge_tick > 100) charge_tick = 0;
        }

        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
          stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (battery_state == BATTERY_STATE_CHARGED) {
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            // stat_area.fillRect(px, py, 14, 3, SSD1306_BLACK);
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawRect(px-2, py-2, 17, 7, SSD1306_WHITE);
            stat_area.drawLine(px+15, py, px+15, py+3, SSD1306_WHITE);
            if (battery_value > 7) stat_area.drawLine(px, py, px, py+2, SSD1306_WHITE);
            if (battery_value > 20) stat_area.drawLine(px+1*2, py, px+1*2, py+2, SSD1306_WHITE);
            if (battery_value > 33) stat_area.drawLine(px+2*2, py, px+2*2, py+2, SSD1306_WHITE);
            if (battery_value > 46) stat_area.drawLine(px+3*2, py, px+3*2, py+2, SSD1306_WHITE);
            if (battery_value > 59) stat_area.drawLine(px+4*2, py, px+4*2, py+2, SSD1306_WHITE);
            if (battery_value > 72) stat_area.drawLine(px+5*2, py, px+5*2, py+2, SSD1306_WHITE);
            if (battery_value > 85) stat_area.drawLine(px+6*2, py, px+6*2, py+2, SSD1306_WHITE);
          }
        }
      } else {
        stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
        stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
      }
    }
  } else {
    stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
    stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
  }
}

#define Q_SNR_STEP 2.0
#define Q_SNR_MIN_BASE -9.0
#define Q_SNR_MAX 6.0
void draw_quality_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);
  if (radio_online) {
    signed char t_snr = (signed int)last_snr_raw;
    int snr_int = (int)t_snr;
    float snr_min = Q_SNR_MIN_BASE-(int)lora_sf*Q_SNR_STEP;
    float snr_span = (Q_SNR_MAX-snr_min);
    float snr = ((int)snr_int) * 0.25;
    float quality = ((snr-snr_min)/(snr_span))*100;
    if (quality > 100.0) quality = 100.0;
    if (quality < 0.0) quality = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (quality > 0)  stat_area.drawLine(px+0*2, py+7, px+0*2, py+6, SSD1306_WHITE);
    if (quality > 15) stat_area.drawLine(px+1*2, py+7, px+1*2, py+5, SSD1306_WHITE);
    if (quality > 30) stat_area.drawLine(px+2*2, py+7, px+2*2, py+4, SSD1306_WHITE);
    if (quality > 45) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (quality > 60) stat_area.drawLine(px+4*2, py+7, px+4*2, py+2, SSD1306_WHITE);
    if (quality > 75) stat_area.drawLine(px+5*2, py+7, px+5*2, py+1, SSD1306_WHITE);
    if (quality > 90) stat_area.drawLine(px+6*2, py+7, px+6*2, py+0, SSD1306_WHITE);
  }
}

#if MODEM == SX1280
  #define S_RSSI_MIN -105.0
  #define S_RSSI_MAX -65.0
#else
  #define S_RSSI_MIN -135.0
  #define S_RSSI_MAX -75.0
#endif
#define S_RSSI_SPAN (S_RSSI_MAX-S_RSSI_MIN)
void draw_signal_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);

  if (radio_online) {
    int rssi_val = last_rssi;
    if (rssi_val < S_RSSI_MIN) rssi_val = S_RSSI_MIN;
    if (rssi_val > S_RSSI_MAX) rssi_val = S_RSSI_MAX;
    int signal = ((rssi_val - S_RSSI_MIN)*(1.0/S_RSSI_SPAN))*100.0;

    if (signal > 100.0) signal = 100.0;
    if (signal < 0.0) signal = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (signal > 85) stat_area.drawLine(px+0*2, py+7, px+0*2, py+0, SSD1306_WHITE);
    if (signal > 72) stat_area.drawLine(px+1*2, py+7, px+1*2, py+1, SSD1306_WHITE);
    if (signal > 59) stat_area.drawLine(px+2*2, py+7, px+2*2, py+2, SSD1306_WHITE);
    if (signal > 46) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (signal > 33) stat_area.drawLine(px+4*2, py+7, px+4*2, py+4, SSD1306_WHITE);
    if (signal > 20) stat_area.drawLine(px+5*2, py+7, px+5*2, py+5, SSD1306_WHITE);
    if (signal > 7)  stat_area.drawLine(px+6*2, py+7, px+6*2, py+6, SSD1306_WHITE);
  }
}

#if MODEM == SX1280
  #define WF_TX_SIZE 5
#else
  #define WF_TX_SIZE 5
#endif
#define WF_RSSI_MAX -60
#define WF_RSSI_MIN -135
#define WF_RSSI_SPAN (WF_RSSI_MAX-WF_RSSI_MIN)
#define WF_PIXEL_WIDTH 10
#define WF_M_RX   0x00
#define WF_M_TX   0x01
#define WF_M_NTFR 0x02
void draw_waterfall(int px, int py) {
  int rssi_val = current_rssi;
  if (rssi_val < WF_RSSI_MIN) rssi_val = WF_RSSI_MIN;
  if (rssi_val > WF_RSSI_MAX) rssi_val = WF_RSSI_MAX;
  int rssi_normalised = ((rssi_val - WF_RSSI_MIN)*(1.0/WF_RSSI_SPAN))*WF_PIXEL_WIDTH;
  if (display_tx) {
    for (uint8_t i = 0; i < WF_TX_SIZE; i++) {
      waterfall_meta[waterfall_head] = WF_M_TX;
      waterfall[waterfall_head++] = -1;
      if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
    }
    display_tx = false;
  } else {
    if (interference_detected) { waterfall_meta[waterfall_head] = WF_M_NTFR; }
    else                       { waterfall_meta[waterfall_head] = WF_M_RX; }
    waterfall[waterfall_head++] = rssi_normalised;
    if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
  }

  stat_area.fillRect(px,py,WF_PIXEL_WIDTH, WATERFALL_SIZE, SSD1306_BLACK);
  for (int i = 0; i < WATERFALL_SIZE; i++){
    int wi = (waterfall_head+i)%WATERFALL_SIZE;
    int ws = waterfall[wi];
    int wm = waterfall_meta[wi];
    if (ws > 0) {
      if      (wm == WF_M_RX)   { stat_area.drawLine(px, py+i, px+ws-1, py+i, SSD1306_WHITE); }
      else if (wm == WF_M_NTFR) {
        uint8_t o = 0;
        for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) { stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE); }
      }
    } else if (ws == -1) {
      uint8_t o = i%2;
      for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) {
        stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE);
      }
    }
  }
}

bool stat_area_intialised = false;
void draw_stat_area() {
  if (device_init_done) {
    if (!stat_area_intialised) {
      stat_area.drawBitmap(0, 0, bm_frame, 64, 64, SSD1306_WHITE, SSD1306_BLACK);
      stat_area_intialised = true;
    }

    draw_cable_icon(3, 8);
    draw_bt_icon(3, 30);
    draw_lora_icon(45, 8);
    draw_mw_icon(45, 30);
    draw_battery_bars(4, 58);
    draw_quality_bars(28, 56);
    draw_signal_bars(44, 56);
    if (radio_online) {
      draw_waterfall(27, 4);
    }
  }
}

void update_stat_area() {
  if (eeprom_ok && !firmware_update_mode && !console_active) {

    draw_stat_area();
    if (disp_mode == DISP_MODE_PORTRAIT) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      drawBitmap(p_as_x+2, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
      if (device_init_done && !disp_ext_fb) drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
    }

  } else {
    if (firmware_update_mode) {
      drawBitmap(p_as_x, p_as_y, bm_updating, stat_area.width(), stat_area.height(), SSD1306_BLACK, SSD1306_WHITE);
    } else if (console_active && device_init_done) {
      drawBitmap(p_as_x, p_as_y, bm_console, stat_area.width(), stat_area.height(), SSD1306_BLACK, SSD1306_WHITE);
      if (disp_mode == DISP_MODE_LANDSCAPE) {
        drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
      }
    }
  }
}

extern char bt_devname[11];
extern char bt_dh[16];
#if HAS_WIFI
  extern IPAddress wr_device_ip;
#endif
void draw_disp_area() {
  if (!device_init_done || firmware_update_mode) {
    uint8_t p_by = 37;
    if (disp_mode == DISP_MODE_LANDSCAPE || firmware_update_mode) {
      p_by = 18;
      disp_area.fillRect(0, 0, disp_area.width(), disp_area.height(), SSD1306_BLACK);
    }
    if (!device_init_done) disp_area.drawBitmap(0, p_by, bm_boot, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
    if (firmware_update_mode) disp_area.drawBitmap(0, p_by, bm_fw_update, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    if (!disp_ext_fb or bt_ssp_pin != 0) {
      if (radio_online && display_diagnostics) {
        disp_area.fillRect(0,8,disp_area.width(),37, SSD1306_BLACK); disp_area.fillRect(0,37,disp_area.width(),27, SSD1306_WHITE);
        disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);

        disp_area.setCursor(2, 13);
        disp_area.print("On");
        disp_area.setCursor(14, 13);
        disp_area.print("@");
        disp_area.setCursor(21, 13);
        disp_area.printf("%.1fKbps", (float)lora_bitrate/1000.0);

        //disp_area.setCursor(31, 23-1);
        disp_area.setCursor(2, 23-1);
        disp_area.print("Airtime:");
        
        disp_area.setCursor(11, 33-1);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", total_channel_util*100.0);
          disp_area.printf("%.1f%%", airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", total_channel_util*100.0);
          disp_area.printf("%.0f%%", airtime*100.0);
        }
        disp_area.drawBitmap(2, 26-1, bm_hg_low, 5, 9, SSD1306_WHITE, SSD1306_BLACK);

        disp_area.setCursor(32+11, 33-1);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_channel_util*100.0);
          disp_area.printf("%.1f%%", longterm_airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_channel_util*100.0);
          disp_area.printf("%.0f%%", longterm_airtime*100.0);
        }
        disp_area.drawBitmap(32+2, 26-1, bm_hg_high, 5, 9, SSD1306_WHITE, SSD1306_BLACK);


        disp_area.setTextColor(SSD1306_BLACK);
        disp_area.setCursor(2, 46);
        disp_area.print("Channel");
        disp_area.setCursor(38, 46);
        disp_area.print("Load:");
        
        disp_area.setCursor(11, 57);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", airtime*100.0);
          disp_area.printf("%.1f%%", total_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", airtime*100.0);
          disp_area.printf("%.0f%%", total_channel_util*100.0);
        }
        disp_area.drawBitmap(2, 50, bm_hg_low, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

        disp_area.setCursor(32+11, 57);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_airtime*100.0);
          disp_area.printf("%.1f%%", longterm_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_airtime*100.0);
          disp_area.printf("%.0f%%", longterm_channel_util*100.0);
        }
        disp_area.drawBitmap(32+2, 50, bm_hg_high, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

      } else {
        if (device_signatures_ok()) { disp_area.drawBitmap(0, 0, bm_def_lc, disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
        else {                        disp_area.drawBitmap(0, 0, bm_def,    disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }

        bool display_ip = false;
        #if HAS_WIFI
          if (wifi_is_connected() && disp_page%2 == 1) { display_ip = true; }
        #endif
        if (display_ip) {
          #if HAS_WIFI
            uint8_t ones = 3+one_counts[wr_device_ip[0]]+one_counts[wr_device_ip[1]]+one_counts[wr_device_ip[2]]+one_counts[wr_device_ip[3]];
            uint8_t chars = 7;
            for (uint8_t i = 0; i<4; i++) { if (wr_device_ip[i] > 9) { chars++; } if (wr_device_ip[i] > 99) { chars++; } }
            uint8_t width = chars*6-(ones*4);
            int alignment_offset = disp_area.width()-width;
            int ipxpos = alignment_offset;
            disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);
            disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK);
            disp_area.setCursor(3, 34-8); disp_area.print("WiFi IP:");
            disp_area.setCursor(ipxpos, 34); disp_area.print(wr_device_ip);
          #endif
        } else {
          disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(2);
          disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK); uint8_t ofsc = 0;
          if ((bt_dh[14] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[14] >> 4)         == 0x01) { ofsc += 8; }
          if ((bt_dh[15] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[15] >> 4)         == 0x01) { ofsc += 8; }
          disp_area.setCursor(17+ofsc, 32); disp_area.printf("%02X%02X", bt_dh[14], bt_dh[15]);
        }
      }

      if (!hw_ready || radio_error || !device_firmware_ok()) {
        if (!device_firmware_ok()) {
          disp_area.drawBitmap(0, 37, bm_fw_corrupt, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (!modem_installed) {
            disp_area.drawBitmap(0, 37, bm_no_radio, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            disp_area.drawBitmap(0, 37, bm_conf_missing, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          }
        }
      } else if (bt_state == BT_STATE_PAIRING and bt_ssp_pin != 0) {
        char *pin_str = (char*)malloc(DISP_PIN_SIZE+1);
        sprintf(pin_str, "%06d", bt_ssp_pin);

        disp_area.drawBitmap(0, 37, bm_pairing, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
        for (int i = 0; i < DISP_PIN_SIZE; i++) {
          uint8_t numeric = pin_str[i]-48;
          uint8_t offset = numeric*5;
          disp_area.drawBitmap(7+9*i, 37+16, bm_n_uh+offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
        }
        free(pin_str);
      } else {
        if (millis()-last_page_flip >= page_interval) {
          disp_page = (++disp_page%pages);
          last_page_flip = millis();
          if (not community_fw and disp_page == 0) disp_page = 1;
        }

        if (radio_online) {
          if (!display_diagnostics) {
            disp_area.drawBitmap(0, 37, bm_online, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          }
        } else {
          if (disp_page == 0) {
            if (true || device_signatures_ok()) {
              disp_area.drawBitmap(0, 37, bm_checks, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            } else {
              disp_area.drawBitmap(0, 37, bm_nfr, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            }
          } else if (disp_page == 1) {
            if (!console_active) {
              disp_area.drawBitmap(0, 37, bm_hwok, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            } else {
              disp_area.drawBitmap(0, 37, bm_console_active, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            }
          } else if (disp_page == 2) {
            disp_area.drawBitmap(0, 37, bm_version, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            char *v_str = (char*)malloc(3+1);
            sprintf(v_str, "%01d%02d", MAJ_VERS, MIN_VERS);
            for (int i = 0; i < 3; i++) {
              uint8_t numeric = v_str[i]-48; uint8_t bm_offset = numeric*5;
              uint8_t dxp = 20;
              if (i == 1) dxp += 9*1+4;
              if (i == 2) dxp += 9*2+4;
              disp_area.drawBitmap(dxp, 37+16, bm_n_uh+bm_offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
            }
            free(v_str);
            disp_area.drawLine(27, 37+19, 28, 37+19, SSD1306_BLACK);
            disp_area.drawLine(27, 37+20, 28, 37+20, SSD1306_BLACK);
          }
        }
      }
    } else {
      disp_area.drawBitmap(0, 0, fb, disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    }
  }
}

void update_disp_area() {
  draw_disp_area();

  drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
  if (disp_mode == DISP_MODE_LANDSCAPE) {
    if (device_init_done && !firmware_update_mode && !disp_ext_fb) {
      drawLine(0, 0, 0, 63, SSD1306_WHITE);
    }
  }
}

void display_recondition() {
  #if PLATFORM == PLATFORM_ESP32
    for (uint8_t iy = 0; iy < disp_area.height(); iy++) {
      unsigned char rand_seg [] = {random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF)};
      stat_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
      disp_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
    }

    drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    if (disp_mode == DISP_MODE_PORTRAIT) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    }
  #endif
}
#else
void display_recondition() { }

#define Q_SNR_STEP 2.0
#define Q_SNR_MIN_BASE -9.0
#define Q_SNR_MAX 6.0
#if MODEM == SX1280
  #define S_RSSI_MIN -105.0
  #define S_RSSI_MAX -65.0
#else
  #define S_RSSI_MIN -135.0
  #define S_RSSI_MAX -75.0
#endif
#define S_RSSI_SPAN (S_RSSI_MAX-S_RSSI_MIN)
#if MODEM == SX1280
  #define WF_TX_SIZE 5
#else
  #define WF_TX_SIZE 5
#endif
#define WF_RSSI_MAX -60
#define WF_RSSI_MIN -135
#define WF_RSSI_SPAN (WF_RSSI_MAX-WF_RSSI_MIN)

extern char bt_devname[11];
extern char bt_dh[16];

uint16_t cp_signal_color(float percent) {
  if (percent > 66.0f) return 0x07E0;
  if (percent > 33.0f) return 0xFBE0;
  return 0xF800;
}

uint16_t cp_load_color(float percent) {
  if (percent < 33.0f) return 0x07E0;
  if (percent < 66.0f) return 0xFBE0;
  return 0xF800;
}

void cp_draw_gradient_bar(int x, int y, int w, int h, float percent, bool load_colours) {
  if (percent > 100.0f) percent = 100.0f;
  if (percent < 0.0f) percent = 0.0f;
  int fill_w = (int)((percent / 100.0f) * (w - 2));
  uint16_t fill_clr = load_colours ? cp_load_color(percent) : cp_signal_color(percent);
  m5canvas.drawRect(x, y, w, h, CLR_BORDER);
  if (fill_w > 0) {
    m5canvas.fillRect(x + 1, y + 1, fill_w, h - 2, fill_clr);
  }
}

void cp_draw_usb_icon(int x, int y, uint16_t colour) {
  m5canvas.fillRect(x + 3, y, 7, 10, colour);
  m5canvas.fillRect(x + 4, y + 10, 5, 4, colour);
  m5canvas.drawPixel(x + 1, y + 3, colour);
  m5canvas.drawPixel(x + 11, y + 3, colour);
}

void cp_draw_bt_icon(int x, int y, uint16_t colour) {
  m5canvas.drawLine(x + 5, y, x + 5, y + 13, colour);
  m5canvas.drawLine(x + 5, y, x + 9, y + 4, colour);
  m5canvas.drawLine(x + 9, y + 4, x + 0, y + 10, colour);
  m5canvas.drawLine(x + 0, y + 3, x + 9, y + 9, colour);
  m5canvas.drawLine(x + 9, y + 9, x + 5, y + 13, colour);
}

void cp_draw_lora_icon(int x, int y, uint16_t colour) {
  m5canvas.fillCircle(x + 5, y + 7, 2, colour);
  m5canvas.drawCircle(x + 5, y + 7, 5, colour);
  m5canvas.drawCircle(x + 5, y + 7, 8, colour);
}

void cp_draw_battery(int x, int y) {
  if (!pmu_ready || !battery_ready || !battery_installed) {
    m5canvas.setTextSize(1);
    m5canvas.setTextColor(CLR_TEXT_DIM);
    m5canvas.setCursor(x, y + 1);
    m5canvas.print("PWR");
    return;
  }

  float bval = battery_percent;
  bool charge_status_known = !battery_indeterminate;
  bool charging = charge_status_known && (battery_state == BATTERY_STATE_CHARGING);
  bool charged = charge_status_known && (battery_state == BATTERY_STATE_CHARGED);
  uint16_t fill_clr = CLR_BAT_FULL;

  if (charged) {
    fill_clr = CLR_BAT_CHARGE;
    bval = 100.0f;
  } else if (charging) {
    fill_clr = CLR_BAT_CHARGE;
    cp_charge_tick += 3;
    if (cp_charge_tick > 100) cp_charge_tick = 0;
    bval = cp_charge_tick;
  } else if (bval > 60.0f) {
    fill_clr = CLR_BAT_FULL;
  } else if (bval > 20.0f) {
    fill_clr = CLR_BAT_MED;
  } else {
    fill_clr = CLR_BAT_LOW;
  }

  int bw = 20;
  int bh = 8;
  m5canvas.drawRect(x, y, bw, bh, CLR_BAT_OUTLINE);
  m5canvas.fillRect(x + bw, y + 2, 2, 4, CLR_BAT_OUTLINE);
  int fill_w = (int)((bval / 100.0f) * (bw - 2));
  if (fill_w > 0) {
    m5canvas.fillRect(x + 1, y + 1, fill_w, bh - 2, fill_clr);
  }

  if (charging) {
    m5canvas.drawLine(x + 10, y + 1, x + 7, y + 5, CLR_TEXT_PRIMARY);
    m5canvas.drawLine(x + 7, y + 5, x + 11, y + 5, CLR_TEXT_PRIMARY);
    m5canvas.drawLine(x + 11, y + 5, x + 8, y + 8, CLR_TEXT_PRIMARY);
  }
}

void cp_draw_statusbar() {
  m5canvas.fillRect(0, 0, DISP_W, 20, CLR_BG_STATUSBAR);

  uint16_t usb_clr = (cable_state == CABLE_STATE_CONNECTED) ? CLR_CABLE_ON : CLR_CABLE_OFF;
  cp_draw_usb_icon(3, 3, usb_clr);

  uint16_t bt_clr = CLR_BT_OFF;
  if (bt_state == BT_STATE_ON) bt_clr = CLR_BT_ON;
  else if (bt_state == BT_STATE_PAIRING) bt_clr = CLR_BT_PAIR;
  else if (bt_state == BT_STATE_CONNECTED) bt_clr = CLR_BT_CONN;
  cp_draw_bt_icon(18, 3, bt_clr);

  cp_draw_lora_icon(35, 3, radio_online ? CLR_LORA_ON : CLR_LORA_OFF);

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(82, 2);
  m5canvas.print("RNode");

  cp_draw_battery(177, 6);

  m5canvas.setFont(nullptr);
  m5canvas.setTextSize(1);
  if (pmu_ready && battery_ready && battery_installed) {
    float pct = battery_percent;
    if (pct > 100.0f) pct = 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    uint16_t pct_clr = CLR_BAT_FULL;
    if (pct <= 20.0f) pct_clr = CLR_BAT_LOW;
    else if (pct <= 60.0f) pct_clr = CLR_BAT_MED;
    m5canvas.setTextColor(pct_clr);
    m5canvas.setCursor(203, 6);
    char bbuf[6];
    snprintf(bbuf, sizeof(bbuf), "%d%%", (int)(pct + 0.5f));
    m5canvas.print(bbuf);
  } else {
    m5canvas.setTextColor(CLR_TEXT_DIM);
    m5canvas.setCursor(203, 6);
    m5canvas.print("--%");
  }

  m5canvas.drawFastHLine(0, 20, DISP_W, CLR_BORDER);
}

void cp_draw_waterfall() {
  if (!radio_online) {
    m5canvas.fillRect(CP_WF_X, CP_WF_Y, CP_WF_W, CP_WF_H, SSD1306_BLACK);
    m5canvas.drawFastVLine(CP_WF_W, CP_WF_Y, CP_WF_H, CLR_BORDER);
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
      cp_waterfall[cp_waterfall_head] = -1;
      cp_waterfall_head = (cp_waterfall_head + 1) % CP_WF_SIZE;
    }
    display_tx = false;
  } else {
    cp_waterfall[cp_waterfall_head] = interference_detected ? 8 : rssi_norm;
    cp_waterfall_head = (cp_waterfall_head + 1) % CP_WF_SIZE;
  }

  m5canvas.fillRect(CP_WF_X, CP_WF_Y, CP_WF_W, CP_WF_H, SSD1306_BLACK);
  for (int i = 0; i < CP_WF_SIZE; i++) {
    int wi = (cp_waterfall_head + i) % CP_WF_SIZE;
    int ws = cp_waterfall[wi];
    int ypos = CP_WF_Y + i;
    if (ws >= 0) {
      uint16_t colour = wf_palette[ws];
      int bar_w = (ws * CP_WF_BAR_W) / 8;
      if (bar_w > 0) {
        m5canvas.drawFastHLine(CP_WF_X + 1, ypos, bar_w, colour);
      }
    } else if (ws == -1) {
      for (int tx = 0; tx < CP_WF_BAR_W; tx += 2) {
        m5canvas.drawPixel(CP_WF_X + 1 + tx, ypos, SSD1306_WHITE);
      }
    }
  }
  m5canvas.drawFastVLine(CP_WF_W, CP_WF_Y, CP_WF_H, CLR_BORDER);
}

void cp_draw_external_framebuffer() {
  int scale = 1;
  int ox = CP_CONTENT_X + (CP_CONTENT_W - 64 * scale) / 2;
  int oy = CP_CONTENT_Y + (CP_CONTENT_H - 64 * scale) / 2;
  m5canvas.fillRect(CP_CONTENT_X, CP_CONTENT_Y, CP_CONTENT_W, CP_CONTENT_H, SSD1306_BLACK);
  m5canvas.drawRect(ox - 2, oy - 2, 64 * scale + 4, 64 * scale + 4, CLR_BORDER);
  for (int row = 0; row < 64; row++) {
    for (int col = 0; col < 64; col++) {
      int index = row * 8 + (col / 8);
      uint8_t bitmask = 1 << (7 - (col % 8));
      if (fb[index] & bitmask) {
        m5canvas.fillRect(ox + col * scale, oy + row * scale, scale, scale, SSD1306_WHITE);
      }
    }
  }
}

void cp_draw_diagnostics() {
  int x = CP_CONTENT_X + 4;
  int y = CP_CONTENT_Y + 2;

  m5canvas.setFont(nullptr);
  m5canvas.setTextSize(2);
  m5canvas.setTextColor(CLR_TEXT_ACCENT);
  m5canvas.setCursor(x, y);
  uint32_t freq_hz = (radio_online && LoRa != nullptr) ? LoRa->getFrequency() : lora_freq;
  char fbuf[16];
  snprintf(fbuf, sizeof(fbuf), "%.3f MHz", (float)freq_hz / 1000000.0f);
  m5canvas.print(fbuf);
  y += 22;

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(x, y);
  uint32_t bw_hz = lora_bw;
  if (bw_hz == 0 && radio_online && LoRa != nullptr) bw_hz = LoRa->getSignalBandwidth();
  char bwstr[10];
  if (bw_hz >= 1000) snprintf(bwstr, sizeof(bwstr), "%luK", (unsigned long)(bw_hz / 1000));
  else snprintf(bwstr, sizeof(bwstr), "%lu", (unsigned long)bw_hz);
  char modbuf[32];
  snprintf(modbuf, sizeof(modbuf), "SF%d %s CR4/%d", lora_sf, bwstr, lora_cr);
  m5canvas.print(modbuf);
  y += 18;

  m5canvas.setTextColor(CLR_TEXT_SECONDARY);
  m5canvas.setCursor(x, y);
  char brbuf[34];
  int txp = (radio_online && LoRa != nullptr) ? (int)LoRa->getTxPower() : (int)lora_txp;
  snprintf(brbuf, sizeof(brbuf), "%.1fKbps  %+ddBm", (float)lora_bitrate / 1000.0f, txp);
  m5canvas.print(brbuf);
  m5canvas.setFont(nullptr);
  y += 16;

  m5canvas.drawFastHLine(x, y, CP_CONTENT_W - 8, CLR_BORDER);
  y += 4;

  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_SECONDARY);
  m5canvas.setCursor(x, y);
  m5canvas.print("Airtime");
  m5canvas.setCursor(x + 110, y);
  m5canvas.print("Channel Load");
  y += 11;

  float at_st = airtime * 100.0f;
  float at_lt = longterm_airtime * 100.0f;
  float cl_st = total_channel_util * 100.0f;
  float cl_lt = longterm_channel_util * 100.0f;
  char abuf[16];
  m5canvas.setCursor(x + 2, y);
  m5canvas.setTextColor(cp_load_color(at_st));
  snprintf(abuf, sizeof(abuf), "ST: %.1f%%", at_st);
  m5canvas.print(abuf);
  m5canvas.setCursor(x + 112, y);
  m5canvas.setTextColor(cp_load_color(cl_st));
  snprintf(abuf, sizeof(abuf), "ST: %.1f%%", cl_st);
  m5canvas.print(abuf);
  y += 10;

  m5canvas.setCursor(x + 2, y);
  m5canvas.setTextColor(cp_load_color(at_lt));
  snprintf(abuf, sizeof(abuf), "LT: %.1f%%", at_lt);
  m5canvas.print(abuf);
  m5canvas.setCursor(x + 112, y);
  m5canvas.setTextColor(cp_load_color(cl_lt));
  snprintf(abuf, sizeof(abuf), "LT: %.1f%%", cl_lt);
  m5canvas.print(abuf);
  y += 14;

  m5canvas.drawFastHLine(x, y, CP_CONTENT_W - 8, CLR_BORDER);
  y += 4;

  signed char t_snr = (signed char)last_snr_raw;
  float snr = ((int)t_snr) * 0.25f;
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(x, y);
  char sigbuf[40];
  snprintf(sigbuf, sizeof(sigbuf), "RSSI:%d  SNR:%.1f", last_rssi, snr);
  m5canvas.print(sigbuf);
  y += 12;

  float rssi_pct = ((float)(last_rssi - S_RSSI_MIN) / S_RSSI_SPAN) * 100.0f;
  cp_draw_gradient_bar(x, y, CP_CONTENT_W - 8, 12, rssi_pct, false);
  y += 15;

  int sf = lora_sf == 0 ? 7 : lora_sf;
  float snr_min = Q_SNR_MIN_BASE - (int)sf * Q_SNR_STEP;
  float snr_span = Q_SNR_MAX - snr_min;
  float quality = snr_span > 0.0f ? ((snr - snr_min) / snr_span) * 100.0f : 0.0f;
  cp_draw_gradient_bar(x, y, CP_CONTENT_W - 8, 12, quality, false);
}

void cp_draw_idle_content() {
  int x = CP_CONTENT_X + 4;
  int y = CP_CONTENT_Y + 8;

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(x, y);
  if (bt_devname[0] != 0) m5canvas.print(bt_devname);
  else m5canvas.print("RNode");
  y += 24;

  m5canvas.setFont(nullptr);
  m5canvas.setTextSize(3);
  m5canvas.setTextColor(CLR_TEXT_ACCENT);
  m5canvas.setCursor(x, y);
  char idbuf[8];
  snprintf(idbuf, sizeof(idbuf), "%02X%02X", (uint8_t)bt_dh[14], (uint8_t)bt_dh[15]);
  m5canvas.print(idbuf);
  y += 34;

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setCursor(x, y);

  if (radio_online) {
    m5canvas.setTextColor(CLR_LORA_ON);
    m5canvas.print("Radio Online");
  } else if (disp_page == 0) {
    #if BOARD_MODEL == BOARD_CARDPUTER_ADV
      m5canvas.setTextColor(CLR_LORA_ON);
      m5canvas.print("Provisioned");
    #else
      if (device_signatures_ok()) {
        m5canvas.setTextColor(CLR_LORA_ON);
        m5canvas.print("Signatures OK");
      } else {
        m5canvas.setTextColor(CLR_TEXT_WARN);
        m5canvas.print("Not Provisioned");
      }
    #endif
  } else if (disp_page == 1) {
    if (!console_active) {
      m5canvas.setTextColor(CLR_LORA_ON);
      m5canvas.print("Hardware OK");
    } else {
      m5canvas.setTextColor(CLR_TEXT_ACCENT);
      m5canvas.print("Console Active");
    }
  } else {
    m5canvas.setTextColor(CLR_TEXT_DIM);
    char vbuf[18];
    snprintf(vbuf, sizeof(vbuf), "Firmware v%d.%02d", MAJ_VERS, MIN_VERS);
    m5canvas.print(vbuf);
  }
  m5canvas.setFont(nullptr);
}

void cp_draw_boot_screen() {
  m5canvas.fillSprite(SSD1306_BLACK);

  m5canvas.setFont(nullptr);
  m5canvas.setTextSize(3);
  m5canvas.setTextColor(CLR_TEXT_ACCENT);
  m5canvas.setCursor(72, 30);
  m5canvas.print("RNode");

  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_SECONDARY);
  m5canvas.setCursor(78, 65);
  m5canvas.print("Cardputer Adv");

  int bar_w = 160;
  int bar_x = (DISP_W - bar_w) / 2;
  int bar_y = 95;
  m5canvas.drawRect(bar_x, bar_y, bar_w, 8, CLR_BORDER);
  int pulse = (millis() / 15) % bar_w;
  int pw = 40;
  int p_start = pulse - pw;
  if (p_start < 0) p_start = 0;
  int p_end = pulse;
  if (p_end > bar_w - 2) p_end = bar_w - 2;
  if (p_end > p_start) {
    m5canvas.fillRect(bar_x + 1 + p_start, bar_y + 1, p_end - p_start, 6, CLR_TEXT_ACCENT);
  }
}

void cp_draw_pairing_screen() {
  int x = CP_CONTENT_X + 4;
  int y = CP_CONTENT_Y + 8;

  m5canvas.setTextSize(2);
  m5canvas.setTextColor(CLR_TEXT_WARN);
  m5canvas.setCursor(x, y);
  m5canvas.print("BT Pairing");
  y += 28;

  if (bt_ssp_pin != 0) {
    char pin_str[7];
    snprintf(pin_str, sizeof(pin_str), "%06d", (int)bt_ssp_pin);

    int dx = x + 4;
    for (int i = 0; i < 6; i++) {
      m5canvas.fillRoundRect(dx, y, 24, 36, 4, CLR_BG_PANEL);
      m5canvas.drawRoundRect(dx, y, 24, 36, 4, CLR_TEXT_ACCENT);
      m5canvas.setTextSize(3);
      m5canvas.setTextColor(CLR_TEXT_ACCENT);
      m5canvas.setCursor(dx + 5, y + 8);
      m5canvas.print(pin_str[i]);
      dx += 30;
    }
  }
}

bool cp_pairing_tip_active() {
  return (int32_t)(cp_pairing_tip_until - millis()) > 0;
}

void cp_draw_pairing_tip_overlay() {
  if (!cp_pairing_tip_active() || bt_state == BT_STATE_PAIRING) return;

  int w = 196;
  int h = 54;
  int x = (DISP_W - w) / 2;
  int y = DISP_H - h - 10;

  m5canvas.fillRoundRect(x + 2, y + 2, w, h, 5, SSD1306_BLACK);
  m5canvas.fillRoundRect(x, y, w, h, 5, CLR_BG_PANEL);
  m5canvas.drawRoundRect(x, y, w, h, 5, CLR_TEXT_ACCENT);

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(x + 12, y + 8);
  m5canvas.print("Press and hold `p`");
  m5canvas.setCursor(x + 12, y + 24);
  m5canvas.print("or OK to enter");
  m5canvas.setCursor(x + 12, y + 40);
  m5canvas.print("pairing mode.");
  m5canvas.setFont(nullptr);
}

bool cp_ble_notice_active() {
  return (int32_t)(cp_ble_notice_until - millis()) > 0;
}

void cp_draw_ble_notice_overlay() {
  if (!cp_ble_notice_active()) return;

  int w = 132;
  int h = 34;
  int x = (DISP_W - w) / 2;
  int y = DISP_H - h - 14;

  m5canvas.fillRoundRect(x + 2, y + 2, w, h, 5, SSD1306_BLACK);
  m5canvas.fillRoundRect(x, y, w, h, 5, CLR_BG_PANEL);
  m5canvas.drawRoundRect(x, y, w, h, 5, cp_ble_notice_enabled ? CLR_BT_ON : CLR_TEXT_DIM);

  m5canvas.setFont(&fonts::Font2);
  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_PRIMARY);
  m5canvas.setCursor(x + 18, y + 10);
  m5canvas.print(cp_ble_notice_enabled ? "BLE Enabled" : "BLE Disabled");
  m5canvas.setFont(nullptr);
}

void cp_draw_fw_update_screen() {
  m5canvas.fillSprite(SSD1306_BLACK);

  m5canvas.setTextSize(2);
  m5canvas.setTextColor(CLR_TEXT_WARN);
  m5canvas.setCursor(20, 30);
  m5canvas.print("Firmware Update");

  int bar_w = 200;
  int bar_x = 20;
  int bar_y = 65;
  m5canvas.drawRect(bar_x, bar_y, bar_w, 10, CLR_BORDER);
  int anim = (millis() / 10) % bar_w;
  int aw = 60;
  int a_end = anim + aw;
  if (a_end > bar_w - 2) a_end = bar_w - 2;
  if (anim < bar_w - 2 && a_end > anim) {
    m5canvas.fillRect(bar_x + 1 + anim, bar_y + 1, a_end - anim, 8, CLR_TEXT_WARN);
  }

  m5canvas.setTextSize(1);
  m5canvas.setTextColor(CLR_TEXT_ERROR);
  m5canvas.setCursor(50, 95);
  m5canvas.print("Do not power off");
}

void cp_draw_error_screen() {
  int x = CP_CONTENT_X + 4;
  int y = CP_CONTENT_Y + 10;

  m5canvas.setTextSize(2);
  m5canvas.setTextColor(CLR_TEXT_ERROR);
  m5canvas.setCursor(x, y);

  if (!device_firmware_ok()) {
    m5canvas.print("FW Corrupt");
    y += 24;
    m5canvas.setTextSize(1);
    m5canvas.setTextColor(CLR_TEXT_SECONDARY);
    m5canvas.setCursor(x, y);
    m5canvas.print("Reflash firmware to fix");
  } else if (!modem_installed) {
    m5canvas.print("No Radio");
    y += 24;
    m5canvas.setTextSize(1);
    m5canvas.setTextColor(CLR_TEXT_SECONDARY);
    m5canvas.setCursor(x, y);
    m5canvas.print("Check radio module");
  } else {
    m5canvas.print("Config Error");
    y += 24;
    m5canvas.setTextSize(1);
    m5canvas.setTextColor(CLR_TEXT_SECONDARY);
    m5canvas.setCursor(x, y);
    m5canvas.print("Run rnodeconf");
  }
}

void cp_render_frame() {
  if (!device_init_done) {
    cp_draw_boot_screen();
    return;
  }

  if (firmware_update_mode) {
    cp_draw_fw_update_screen();
    return;
  }

  if (millis() - last_page_flip >= page_interval) {
    disp_page = (++disp_page % pages);
    last_page_flip = millis();
    if (!community_fw && disp_page == 0) disp_page = 1;
  }

  m5canvas.fillRect(CP_CONTENT_X, CP_CONTENT_Y, CP_CONTENT_W, CP_CONTENT_H, SSD1306_BLACK);
  cp_draw_statusbar();
  cp_draw_waterfall();

  if (disp_ext_fb && bt_ssp_pin == 0) {
    cp_draw_external_framebuffer();
  } else if (!hw_ready || radio_error || !device_firmware_ok()) {
    cp_draw_error_screen();
  } else if (bt_state == BT_STATE_PAIRING && bt_ssp_pin != 0) {
    cp_draw_pairing_screen();
  } else if (radio_online && display_diagnostics) {
    cp_draw_diagnostics();
  } else {
    cp_draw_idle_content();
  }

  cp_draw_pairing_tip_overlay();
  cp_draw_ble_notice_overlay();
}
#endif

#if BOARD_MODEL == BOARD_TDECK
  #include "TDeckUI.h"
#endif

bool epd_blanked = false;
#if BOARD_MODEL == BOARD_TECHO
  void epd_blank(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_WHITE);
    display.display(full_update);
  }

  void epd_black(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_BLACK);
    display.display(full_update);
  }
#endif

void update_display(bool blank = false) {
  display_updating = true;
  #if BOARD_MODEL == BOARD_TDECK
    if (td_ui_ready) td_poll_touch();
  #endif
  if (blank == true) {
    last_disp_update = millis()-disp_update_interval-1;
  } else {
    if (display_blanking_enabled && millis()-last_unblank_event >= display_blanking_timeout) {
      blank = true;
      display_blanked = true;
      if (display_intensity != 0) {
        display_unblank_intensity = display_intensity;
      }
      display_intensity = 0;
    } else {
      display_blanked = false;
      if (display_unblank_intensity != 0x00) {
        display_intensity = display_unblank_intensity;
        display_unblank_intensity = 0x00;
      }
    }
  }

  if (blank) {
    if (display_blank_frame_drawn && display_contrast == display_intensity) {
      display_updating = false;
      return;
    }
    if (millis()-last_disp_update >= disp_update_interval) {
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        #if BOARD_MODEL == BOARD_CARDPUTER_ADV
          set_contrast(&m5canvas, display_contrast);
        #else
          set_contrast(&display, display_contrast);
        #endif
      }

      #if BOARD_MODEL == BOARD_TECHO
        if (!epd_blanked) {
          epd_blank();
          epd_blanked = true;
        }
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        display.display();
        digitalWrite(PIN_T114_TFT_BLGT, HIGH);
      #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
        m5canvas.fillSprite(SSD1306_BLACK);
        m5canvas.pushSprite(0, 0);
      #elif BOARD_MODEL == BOARD_TDECK
        display.fillScreen(SSD1306_BLACK);
      #elif BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
        display.display();
      #endif

      display_blank_frame_drawn = true;
      last_disp_update = millis();
    }

  } else {
    display_blank_frame_drawn = false;
    if (millis()-last_disp_update >= disp_update_interval) {
      uint32_t current = millis();
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        #if BOARD_MODEL == BOARD_CARDPUTER_ADV
          set_contrast(&m5canvas, display_contrast);
        #else
          set_contrast(&display, display_contrast);
        #endif
      }

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
        m5canvas.fillSprite(SSD1306_BLACK);
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
      #endif

      if (recondition_display) {
        disp_target_fps = 30;
        disp_update_interval = 1000/disp_target_fps;
        display_recondition();
      } else {
        #if BOARD_MODEL == BOARD_TECHO
          display.setFullWindow();
          display.fillScreen(SSD1306_WHITE);
        #endif

        #if BOARD_MODEL == BOARD_CARDPUTER_ADV
          cp_render_frame();
        #elif BOARD_MODEL == BOARD_TDECK
          td_render_frame();
        #else
          update_stat_area();
          update_disp_area();
        #endif
      }
      
      #if BOARD_MODEL == BOARD_TECHO
        if (current-last_epd_refresh >= epd_update_interval) {
          if (current-last_epd_full_refresh >= REFRESH_PERIOD) { display.display(false); last_epd_full_refresh = millis(); }
          else { display.display(true); }
          last_epd_refresh = millis();
          epd_blanked = false;
        }
      #elif BOARD_MODEL == BOARD_CARDPUTER_ADV
        m5canvas.pushSprite(0, 0);
      #elif BOARD_MODEL == BOARD_TDECK
        td_push_frame();
      #elif BOARD_MODEL != BOARD_TDECK
        display.display();
      #endif

      last_disp_update = millis();
    }
  }
  display_updating = false;
}

void display_unblank() {
  last_unblank_event = millis();
  display_blank_frame_drawn = false;
}

#if BOARD_MODEL == BOARD_CARDPUTER_ADV
void cardputer_show_pairing_tip() {
  cp_pairing_tip_until = millis() + 3500;
  display_unblank();
  last_disp_update = 0;
}

void cardputer_show_ble_notice(bool enabled) {
  cp_ble_notice_enabled = enabled;
  cp_ble_notice_until = millis() + 2200;
  display_unblank();
  last_disp_update = 0;
}
#endif

void ext_fb_enable() {
  disp_ext_fb = true;
}

void ext_fb_disable() {
  disp_ext_fb = false;
}
