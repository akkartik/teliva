#ifndef __TELIVA_H__
#define __TELIVA_H__

// Some names for hotkeys beyond those provided by ncurses.

enum KEY_ACTION {
  KEY_NULL = 0,
  CTRL_A = 1,
  CTRL_B = 2,
  CTRL_C = 3,
  CTRL_D = 4,
  CTRL_E = 5,
  CTRL_F = 6,
  CTRL_G = 7,
  CTRL_H = 8,
  TAB = 9,
  ENTER = 10,
  CTRL_K = 11,
  CTRL_L = 12,
  CTRL_Q = 17,
  CTRL_R = 18,
  CTRL_S = 19,
  CTRL_U = 21,
  CTRL_X = 24,
  CTRL_SLASH = 31,
  CTRL_UNDERSCORE = 31,
  DELETE = 127,
};


// Colors (experimental)
// Primary goal here: Keep text readable regardless of OS, terminal emulator
// and color scheme. Unfortunately I don't yet have a good answer, so this
// approach may yet change. Current approach:
//  - Hardcode colors so that we can be sure we use legible combinations of
//    foreground and background.
//  - Use only the terminal palette in the range 16-255.
//    - Not all terminals may support more than 256 colors. (I'm not yet sure
//      everyone has even 256 colors. If you don't, please let me know:
//      http://akkartik.name/contact)
//    - Many terminals provide color schemes which give the ability to tweak
//      colors 0-15. This makes it hard to assume specific combinations are
//      legible. I'm hoping most terminal emulators don't tend to encourage
//      messing with colors 16-255. (Please let me know if you know of
//      counter-examples.)
//
// For now, you have to edit these values if you want to adjust colors in the
// editing environment. Check out https://www.robmeerman.co.uk/unix/256colours
// for a map of available colors.

// Toggle between a few color schemes
#define COLOR_SCHEME 0
#if COLOR_SCHEME == 0
// Light color scheme.
enum color {
  COLOR_FOREGROUND = 238,                     // almost black
  COLOR_BACKGROUND = 253,                     // almost white
  COLOR_FADE = 244,                           // closer to background
  COLOR_MENU_ALTERNATE = 248,
  COLOR_HIGHLIGHT_FOREGROUND = 238,
  COLOR_HIGHLIGHT_BACKGROUND = 250,
  COLOR_ERROR_FOREGROUND = COLOR_BACKGROUND,
  COLOR_ERROR_BACKGROUND = 124,               // deep red
  COLOR_LUA_COMMENT = 27,                     // blue
  COLOR_LUA_KEYWORD = 172,                    // orange
  COLOR_LUA_CONSTANT = 31,                    // cyan
  COLOR_MATCH_FOREGROUND = COLOR_BACKGROUND,
  COLOR_MATCH_BACKGROUND = 28,                // green
};
#elif COLOR_SCHEME == 1
// Dark color scheme.
enum color {
  COLOR_FOREGROUND = 253,                     // almost white
  COLOR_BACKGROUND = 238,                     // almost black
  COLOR_FADE = 244,                           // closer to background
  COLOR_MENU_ALTERNATE = 244,
  COLOR_HIGHLIGHT_FOREGROUND = 238,
  COLOR_HIGHLIGHT_BACKGROUND = 250,
  COLOR_ERROR_FOREGROUND = COLOR_BACKGROUND,
  COLOR_ERROR_BACKGROUND = 124,               // deep red
  COLOR_LUA_COMMENT = 39,                     // blue
  COLOR_LUA_KEYWORD = 172,                    // orange
  COLOR_LUA_CONSTANT = 37,                    // cyan
  COLOR_MATCH_FOREGROUND = COLOR_BACKGROUND,
  COLOR_MATCH_BACKGROUND = 28,                // green
};
#elif COLOR_SCHEME == 2
// Solarized dark.
enum color {
  COLOR_FOREGROUND = 250,                     // almost white
  COLOR_BACKGROUND = 24,                      // dark blue-green
  COLOR_FADE = 246,                           // closer to background
  COLOR_MENU_ALTERNATE = 244,
  COLOR_HIGHLIGHT_FOREGROUND = 250,
  COLOR_HIGHLIGHT_BACKGROUND = 31,
  COLOR_ERROR_FOREGROUND = 250,
  COLOR_ERROR_BACKGROUND = 124,               // deep red
  COLOR_LUA_COMMENT = 45,                     // light blue
  COLOR_LUA_KEYWORD = 172,                    // orange
  COLOR_LUA_CONSTANT = 37,                    // cyan
  COLOR_MATCH_FOREGROUND = COLOR_FOREGROUND,
  COLOR_MATCH_BACKGROUND = 125,               // magenta
};
#endif

enum color_pair {
  COLOR_PAIR_NORMAL = 0,
  COLOR_PAIR_HIGHLIGHT = 1,
  COLOR_PAIR_FADE = 2,
  COLOR_PAIR_MENU_ALTERNATE = 3,
  COLOR_PAIR_LUA_COMMENT = 4,
  COLOR_PAIR_LUA_KEYWORD = 5,
  COLOR_PAIR_LUA_CONSTANT = 6,
  COLOR_PAIR_MATCH = 7,
  COLOR_PAIR_ERROR = 255,
};

#endif
