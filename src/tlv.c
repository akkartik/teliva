#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "lua.h"
#include "lauxlib.h"

void teliva_load_multiline_string(lua_State* L, FILE* in) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  char line[1024] = {'\0'};
  int indent = -1;
  while (!feof(in)) {
    char c = fgetc(in);
    ungetc(c, in);
    if (c != ' ') break;
    assert(fgets(line, 1024, in));
    assert(line[strlen(line)-1] == '\n');
    char* start = strchr(line, '>');
    if (indent == -1)
      indent = start - line;
    else
      assert(indent == start - line);
    ++start;  /* skip '>' */
    luaL_addstring(&b, start);
  }
  luaL_pushresult(&b);
}

/* leave a single table on stack containing the next top-level definition from the file */
void teliva_load_definition(lua_State* L, FILE* in) {
  lua_newtable(L);
  int def_idx = lua_gettop(L);
  char line[1024] = {'\0'};
  char key[512] = {'\0'};
  char value[1024] = {'\0'};
  while (!feof(in)) {
    assert(fgets(line, 1024, in));
    assert(line[strlen(line)-1] == '\n');
    if (line[0] == '#') continue;  /* comment */
    assert(line[0] == '-' || line[0] == ' ');
    assert(line[1] == ' ');
    memset(key, 0, 512);
    memset(value, 0, 1024);
    sscanf(line+2, "%s%s", key, value);
    assert(key[strlen(key)-1] == ':');
    key[strlen(key)-1] = '\0';
    lua_pushstring(L, key);
    if (value[0] != '\0')
      lua_pushstring(L, value);  /* value string on same line */
    else
      teliva_load_multiline_string(L, in);  /* load from later lines */
    lua_settable(L, def_idx);
    /* done with this definition? */
    char c = fgetc(in);
    ungetc(c, in);
    if (c != ' ') break;
  }
}

void load_tlv(lua_State* L, char* filename) {
  lua_newtable(L);
  int history_array = lua_gettop(L);
  FILE* in = fopen(filename, "r");
  for (int i = 1; !feof(in); ++i) {
    teliva_load_definition(L, in);
    if (lua_isnil(L, -1)) break;
    lua_rawseti(L, history_array, i);
  }
  fclose(in);
  lua_setglobal(L, "teliva_program");
}
