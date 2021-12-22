#include <ncurses.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "teliva.h"


int menu_column = 0;
void draw_string_on_menu (const char* s) {
  mvaddstr(LINES-1, menu_column, " ");
  ++menu_column;
  mvaddstr(LINES-1, menu_column, s);
  menu_column += strlen(s);
  mvaddstr(LINES-1, menu_column, " ");
  ++menu_column;
}
void draw_menu_item (const char* key, const char* name) {
  attroff(A_REVERSE);
  draw_string_on_menu(key);
  attron(A_REVERSE);
  draw_string_on_menu(name);
}

void draw_menu (lua_State *L) {
  attron(A_BOLD|A_REVERSE|COLOR_PAIR(COLOR_PAIR_MENU));
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  menu_column = 2;
  draw_menu_item("^x", "exit");
  draw_menu_item("^e", "edit");

  /* render any app-specific items */
  lua_getglobal(L, "menu");
  int table = lua_gettop(L);
  if (lua_istable(L, -1)) {
    for (int i = 1; i <= luaL_getn(L, table); ++i) {
      lua_rawgeti(L, table, i);
      int menu_item = lua_gettop(L);
      lua_rawgeti(L, menu_item, 1);  /* key */
      lua_rawgeti(L, menu_item, 2);  /* value */
      draw_menu_item(lua_tostring(L, -2), lua_tostring(L, -1));
      lua_pop(L, 3);
    }
  }

  lua_pop(L, 1);
  attrset(A_NORMAL);
}
