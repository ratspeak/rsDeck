#include "Theme.h"

// Dark column is the original Ratdeck palette, verbatim. Light column is
// derived: same hue family, brand greens/cyan darkened for light-bg contrast.
#define THEME_COLORS(X) \
    X(BG,             0x05080A, 0xF2F6F4) \
    X(BG_ELEVATED,    0x0C1418, 0xFFFFFF) \
    X(BG_SURFACE,     0x121D23, 0xE4ECE9) \
    X(BG_HOVER,       0x183033, 0xD5E5DE) \
    X(TEXT_PRIMARY,   0xE4F3F0, 0x0E1F1B) \
    X(TEXT_SECONDARY, 0x91A8A5, 0x47615C) \
    X(TEXT_MUTED,     0x526866, 0x8CA29D) \
    X(PRIMARY,        0x00E06D, 0x008F4C) \
    X(PRIMARY_MUTED,  0x00A853, 0x00713C) \
    X(PRIMARY_SUBTLE, 0x06251A, 0xD7F0E1) \
    X(ACCENT,         0x4FD7FF, 0x0077A8) \
    X(SUCCESS,        0x31E981, 0x168A4D) \
    X(WARNING_CLR,    0xF0C04F, 0x9A7400) \
    X(ERROR_CLR,      0xFF5C6C, 0xD02E44) \
    X(ERROR_SUBTLE,   0x2A0E0E, 0xF6DBDD) \
    X(BORDER,         0x21383B, 0xC2D3CE) \
    X(BORDER_ACTIVE,  0x00E06D, 0x008F4C) \
    X(DIVIDER,        0x102329, 0xDCE6E2) \
    X(MSG_OUT_BG,     0x082719, 0xD7F0E1) \
    X(MSG_IN_BG,      0x0B151C, 0xE7EDF1) \
    X(STATUS_BG,      0x071014, 0xE4ECE9) \
    X(STATUS_FLASH,   0x12351F, 0xBFE8CE) \
    X(TAB_BG,         0x081115, 0xE4ECE9) \
    X(TAB_ACTIVE_BG,  0x0B241B, 0xD7F0E1) \
    X(TAB_ACTIVE,     0x00E06D, 0x00713C) \
    X(TAB_INACTIVE,   0x91A8A5, 0x47615C) \
    X(BADGE_BG,       0xFF5C6C, 0xD02E44) \
    X(TOAST_BG,       0x00E06D, 0x008F4C)

namespace Theme {

namespace {
Scheme g_scheme = Scheme::DARK;
}

#define THEME_DEFINE(name, dark, light) uint32_t name = dark;
THEME_COLORS(THEME_DEFINE)
#undef THEME_DEFINE

void setScheme(Scheme s) {
    g_scheme = s;
    const bool light = (s == Scheme::LIGHT);
#define THEME_APPLY(name, d, l) name = light ? (uint32_t)(l) : (uint32_t)(d);
    THEME_COLORS(THEME_APPLY)
#undef THEME_APPLY
}

Scheme scheme() { return g_scheme; }

}  // namespace Theme
