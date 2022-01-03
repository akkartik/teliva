#ifndef __TELIVA_H__
#define __TELIVA_H__

/*** Some details for Teliva apps to be aware of. */

/* Some names for hotkeys beyond those provided by ncurses. */
/* TODO: expose these in the curses wrappers. */

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
  CTRL_N = 14,
  CTRL_P = 16,
  CTRL_Q = 17,
  CTRL_R = 18,
  CTRL_S = 19,
  CTRL_U = 21,
  CTRL_X = 24,
  CTRL_SLASH = 31,
  CTRL_UNDERSCORE = 31,
  DELETE = 127,
};


/* Colors (experimental)
 * Primary goal here: Keep text readable regardless of OS, terminal emulator
 * and color scheme. Unfortunately I don't yet have a good answer, so this
 * approach may yet change. Current approach:
 *  - Hardcode colors so that we can be sure we use legible combinations of
 *    foreground and background.
 *  - Use only the terminal palette in the range 16-255.
 *    - Not all terminals may support more than 256 colors. (I'm not yet sure
 *      everyone has even 256 colors. If you don't, please let me know:
 *      http://akkartik.name/contact)
 *    - Many terminals provide color schemes which give the ability to tweak
 *      colors 0-15. This makes it hard to assume specific combinations are
 *      legible. I'm hoping most terminal emulators don't tend to encourage
 *      messing with colors 16-255. (Please let me know if you know of
 *      counter-examples.)
 *
 * For now, you have to edit these values if you want to adjust colors in the
 * editing environment. Check out https://www.robmeerman.co.uk/unix/256colours
 * for a map of available colors. */

/* Toggle between a few color schemes */
#define COLOR_SCHEME 0
#if COLOR_SCHEME == 0
/* Light color scheme. */
enum color {
  COLOR_FOREGROUND = 238,                     /* almost black */
  COLOR_BACKGROUND = 253,                     /* almost white */
  COLOR_FADE = 244,                           /* closer to background */
  COLOR_MENU_ALTERNATE = 248,
  COLOR_SELECTABLE_FOREGROUND = 238,
  COLOR_SELECTABLE_BACKGROUND = 250,
  COLOR_ERROR_FOREGROUND = COLOR_BACKGROUND,
  COLOR_ERROR_BACKGROUND = 124,               /* deep red */
  COLOR_SAFE_NORMAL = 28,                     /* green */
  COLOR_SAFE_REVERSE = 46,                    /* green */
  COLOR_WARN_NORMAL = 208,                    /* orange */
  COLOR_WARN_REVERSE = 208,                   /* orange */
  COLOR_RISK_NORMAL = 196,                    /* red */
  COLOR_RISK_REVERSE = 196,                   /* red */
  COLOR_LUA_COMMENT = 27,                     /* blue */
  COLOR_LUA_KEYWORD = 172,                    /* orange */
  COLOR_LUA_CONSTANT = 31,                    /* cyan */
  COLOR_MATCH_FOREGROUND = COLOR_BACKGROUND,
  COLOR_MATCH_BACKGROUND = 28,                /* green */
};
#elif COLOR_SCHEME == 1
/* Dark color scheme. */
enum color {
  COLOR_FOREGROUND = 253,                     /* almost white */
  COLOR_BACKGROUND = 238,                     /* almost black */
  COLOR_FADE = 244,                           /* closer to background */
  COLOR_MENU_ALTERNATE = 244,
  COLOR_SELECTABLE_FOREGROUND = 238,
  COLOR_SELECTABLE_BACKGROUND = 250,
  COLOR_ERROR_FOREGROUND = COLOR_FOREGROUND,
  COLOR_ERROR_BACKGROUND = 124,               /* deep red */
  COLOR_SAFE_NORMAL = 46,                     /* green */
  COLOR_SAFE_REVERSE = 28,                    /* green */
  COLOR_WARN_NORMAL = 208,                    /* orange */
  COLOR_WARN_REVERSE = 130,                   /* orange */
  COLOR_RISK_NORMAL = 196,                    /* red */
  COLOR_RISK_REVERSE = 196,                   /* red */
  COLOR_LUA_COMMENT = 39,                     /* blue */
  COLOR_LUA_KEYWORD = 172,                    /* orange */
  COLOR_LUA_CONSTANT = 37,                    /* cyan */
  COLOR_MATCH_FOREGROUND = COLOR_BACKGROUND,
  COLOR_MATCH_BACKGROUND = 28,                /* green */
};
#elif COLOR_SCHEME == 2
/* Solarized dark. */
enum color {
  COLOR_FOREGROUND = 250,                     /* almost white */
  COLOR_BACKGROUND = 24,                      /* dark blue-green */
  COLOR_FADE = 246,                           /* closer to background */
  COLOR_MENU_ALTERNATE = 244,
  COLOR_SELECTABLE_FOREGROUND = 250,
  COLOR_SELECTABLE_BACKGROUND = 31,
  COLOR_ERROR_FOREGROUND = 250,
  COLOR_ERROR_BACKGROUND = 124,               /* deep red */
  COLOR_SAFE_NORMAL = 46,                     /* green */
  COLOR_SAFE_REVERSE = 28,                    /* green */
  COLOR_WARN_NORMAL = 208,                    /* orange */
  COLOR_WARN_REVERSE = 130,                   /* orange */
  COLOR_RISK_NORMAL = 201,                    /* red */
  COLOR_RISK_REVERSE = 196,                   /* red */
  COLOR_LUA_COMMENT = 45,                     /* light blue */
  COLOR_LUA_KEYWORD = 172,                    /* orange */
  COLOR_LUA_CONSTANT = 37,                    /* cyan */
  COLOR_MATCH_FOREGROUND = COLOR_FOREGROUND,
  COLOR_MATCH_BACKGROUND = 125,               /* magenta */
};
#endif

enum color_pair {
  COLOR_PAIR_NORMAL = 0,
  COLOR_PAIR_SELECTABLE = 1,
  COLOR_PAIR_FADE = 2,
  COLOR_PAIR_MENU_ALTERNATE = 3,
  COLOR_PAIR_LUA_COMMENT = 4,
  COLOR_PAIR_LUA_KEYWORD = 5,
  COLOR_PAIR_LUA_CONSTANT = 6,
  COLOR_PAIR_MATCH = 7,
  COLOR_PAIR_SAFE = 251,  /* reserved for teliva; apps shouldn't use it */
  COLOR_PAIR_WARN = 252,  /* reserved for teliva; apps shouldn't use it */
  COLOR_PAIR_RISK = 253,  /* reserved for teliva; apps shouldn't use it */
  COLOR_PAIR_MENU = 254,  /* reserved for teliva; apps shouldn't use it */
  COLOR_PAIR_ERROR = 255,  /* reserved for teliva; apps shouldn't use it */
};

/*** C Interface */

/* Each category of primitives below shows a few options from high to low
 * levels of abstraction.
 * (Lower levels aren't complete or well-designed, just what code outside
 * teliva.c needs.) */

/* Integrate with Lua VM */
extern char** Argv;
extern char* Previous_message;
extern int handle_image(lua_State* L, char** argv, int n);
extern void developer_mode(lua_State* L);
extern void permissions_mode(lua_State* L);
extern int file_operation_permitted(const char* filename, const char* mode);
extern int net_operations_permitted;

extern int load_editor_buffer_to_current_definition_in_image(lua_State* L);
extern void save_to_current_definition_and_editor_buffer(lua_State* L, const char* definition);
extern void save_editor_state(int rowoff, int coloff, int cy, int cx);

extern void assign_call_graph_depth_to_name(lua_State* L, int depth, const char* name);
extern void append_to_audit_log(lua_State* L, const char* buffer);

/* Standard UI elements */
extern void render_trusted_teliva_data(lua_State* L);

extern void draw_menu_item(const char* key, const char* name);

extern void draw_string_on_menu(const char* s);

extern int menu_column;

/* Error reporting */

extern const char* Previous_error;
extern int report_in_developer_mode(lua_State* L, int status);

extern void render_previous_error(void);

#endif
