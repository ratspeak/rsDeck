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

#ifndef INPUT_H
  #define INPUT_H

  #if BOARD_MODEL == BOARD_CARDPUTER_ADV
    #include <M5Cardputer.h>
  #endif
  
  #define PIN_BUTTON pin_btn_usr1

  #define PRESSED LOW
  #define RELEASED HIGH

  #define EVENT_ALL                 0x00
  #define EVENT_CLICKS              0x01
  #define EVENT_BUTTON_DOWN         0x11
  #define EVENT_BUTTON_UP           0x12
  #define EVENT_BUTTON_CLICK        0x13
  #define EVENT_BUTTON_DOUBLE_CLICK 0x14
  #define EVENT_BUTTON_TRIPLE_CLICK 0x15
  
  int button_events = EVENT_CLICKS;
  int button_state = RELEASED;
  int debounce_state = button_state;
  unsigned long button_debounce_last = 0;
  unsigned long button_debounce_delay = 25;
  unsigned long button_down_last = 0;
  unsigned long button_up_last = 0;

  #if BOARD_MODEL == BOARD_CARDPUTER_ADV
    #define CARDPUTER_PAIR_HOLD_MS 3000
    bool cardputer_enter_down = false;
    bool cardputer_enter_pairing_sent = false;
    unsigned long cardputer_enter_down_last = 0;
    bool cardputer_p_down = false;
    bool cardputer_p_pairing_sent = false;
    unsigned long cardputer_p_down_last = 0;
    bool cardputer_b_down = false;
    bool cardputer_b_toggle_sent = false;
    unsigned long cardputer_b_down_last = 0;
  #endif

  // Forward declaration
  void button_event(uint8_t event, unsigned long duration);
  #if BOARD_MODEL == BOARD_CARDPUTER_ADV
    void cardputer_ble_toggle_event();
  #endif

  #if BOARD_MODEL == BOARD_CARDPUTER_ADV && HAS_DISPLAY
    void cardputer_show_pairing_tip();
  #endif

  void input_init() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
  }

  void input_get_all_events() {
    button_events = EVENT_ALL;
  }

  void input_get_click_events() {
    button_events = EVENT_CLICKS;
  }

  void input_read() {
    int button_reading = digitalRead(PIN_BUTTON);
    if (button_reading != debounce_state) {
      button_debounce_last = millis();
      debounce_state = button_reading;
    }

    if ((millis() - button_debounce_last) > button_debounce_delay) {
      if (button_reading != button_state) {
        // State changed
        int previous_state = button_state;
        button_state = button_reading;

        if (button_events == EVENT_ALL) {
          if (button_state == PRESSED) {
            button_event(EVENT_BUTTON_DOWN, 0);
          } else if (button_state == RELEASED) {
            button_event(EVENT_BUTTON_UP, 0);
          }
        } else if (button_events == EVENT_CLICKS) {
          if (previous_state == PRESSED && button_state == RELEASED) {
            button_up_last = millis();
            button_event(EVENT_BUTTON_CLICK, button_up_last-button_down_last);
          } else if (previous_state == RELEASED && button_state == PRESSED) {
            button_down_last = millis();
          }
        }
      }
    }

    #if BOARD_MODEL == BOARD_CARDPUTER_ADV
      M5Cardputer.update();
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      bool keyboard_changed = M5Cardputer.Keyboard.isChange();
      bool keyboard_pressed = M5Cardputer.Keyboard.isPressed() > 0;
      bool display_was_blanked = display_blanked;
      bool p_pressed = false;
      bool b_pressed = false;

      for (auto key : status.word) {
        if (key == 'p' || key == 'P') {
          p_pressed = true;
        } else if (key == 'b' || key == 'B') {
          b_pressed = true;
        }
      }

      if (keyboard_pressed) {
        display_unblank();
      }

      if (status.enter) {
        if (!cardputer_enter_down) {
          cardputer_enter_down = true;
          cardputer_enter_pairing_sent = false;
          cardputer_enter_down_last = millis();
        } else if (!cardputer_enter_pairing_sent && millis() - cardputer_enter_down_last >= CARDPUTER_PAIR_HOLD_MS) {
          button_event(EVENT_BUTTON_CLICK, 5500);
          cardputer_enter_pairing_sent = true;
        }
      } else {
        cardputer_enter_down = false;
        cardputer_enter_pairing_sent = false;
      }

      if (p_pressed) {
        if (!cardputer_p_down) {
          cardputer_p_down = true;
          cardputer_p_pairing_sent = false;
          cardputer_p_down_last = millis();
        } else if (!cardputer_p_pairing_sent && millis() - cardputer_p_down_last >= CARDPUTER_PAIR_HOLD_MS) {
          button_event(EVENT_BUTTON_CLICK, 5500);
          cardputer_p_pairing_sent = true;
        }
      } else {
        cardputer_p_down = false;
        cardputer_p_pairing_sent = false;
      }

      if (b_pressed) {
        if (!cardputer_b_down) {
          cardputer_b_down = true;
          cardputer_b_toggle_sent = false;
          cardputer_b_down_last = millis();
        } else if (!cardputer_b_toggle_sent && millis() - cardputer_b_down_last >= CARDPUTER_PAIR_HOLD_MS) {
          cardputer_ble_toggle_event();
          cardputer_b_toggle_sent = true;
        }
      } else {
        cardputer_b_down = false;
        cardputer_b_toggle_sent = false;
      }

      if (keyboard_changed && keyboard_pressed && !display_was_blanked && !status.enter && !p_pressed && !b_pressed) {
        cardputer_show_pairing_tip();
      }
    #endif
  }

  bool button_pressed() {
    if (button_state == PRESSED) {
      return true;
    } else {
      return false;
    }
  }

#endif
