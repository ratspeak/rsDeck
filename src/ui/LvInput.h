#pragma once

#include <lvgl.h>

class Keyboard;
class Trackball;
class TouchInput;
struct KeyEvent;

// LVGL input device drivers for T-Deck Plus hardware
namespace LvInput {

void init(Keyboard* kb, Trackball* tb, TouchInput* touch);

// Feed a KeyEvent into the LVGL keypad indev (called from main loop)
void feedKey(const KeyEvent& evt);

// Get the LVGL input group (for focusing widgets)
lv_group_t* group();

// Re-apply palette colors to the touch cursor after a theme switch
void applyTheme();

}  // namespace LvInput
