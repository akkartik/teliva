#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "lua.h"
#include "lauxlib.h"

/* If you encounter assertion failures in this file and _didn't_ manually edit
 * it, lease report the .tlv file that caused them: http://akkartik.name/contact. */

void teliva_load_multiline_string(lua_State* L, FILE* in, char* line, int capacity) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  int expected_indent = -1;
  while (1) {
    if (feof(in)) break;
    memset(line, '\0', capacity);
    if (fgets(line, capacity, in) == NULL) break;  /* eof */
    int max = strlen(line);
    assert(line[max-1] == '\n');
    int indent = 0;
    while (indent < max-1 && line[indent] == ' ')
      ++indent;
    if (line[indent] != '>') break;
    if (expected_indent == -1)
      expected_indent = indent;
    else
      assert(expected_indent == indent);
    int start = indent+1;  /* skip '>' */
    luaL_addstring(&b, &line[start]);  /* guaranteed to at least be null */
  }
  luaL_pushresult(&b);
  /* final state of line goes out into the world */
}

/* leave a single table on stack containing the next top-level definition from the file */
void teliva_load_definition(lua_State* L, FILE* in) {
  lua_newtable(L);
  int def_idx = lua_gettop(L);
  char line[1024] = {'\0'};
  do {
    if (feof(in) || fgets(line, 1024, in) == NULL) {
      lua_pushnil(L);
      return;
    }
  } while (line[0] == '#');  /* comment at start of file */
  assert(line[strlen(line)-1] == '\n');
  do {
    assert(line[0] == '-' || line[0] == ' ');
    assert(line[1] == ' ');
    /* key/value pair always indented at 0, never empty, unambiguously not a multiline string */
    char key[512] = {'\0'};
    char value[1024] = {'\0'};
    assert(line[2] != ' ');
    assert(line[2] != '>');
    assert(line[2] != '\n');
    assert(line[2] != '\0');
    memset(key, 0, 512);
    memset(value, 0, 1024);
    sscanf(line+2, "%s%s", key, value);
    assert(key[strlen(key)-1] == ':');
    key[strlen(key)-1] = '\0';
    lua_pushstring(L, key);
    if (value[0] != '\0') {
      lua_pushstring(L, value);  /* value string on same line */
      assert(fgets(line, 1024, in));
    }
    else {
      teliva_load_multiline_string(L, in, line, 1024);  /* load from later lines */
    }
    lua_settable(L, def_idx);
  } while (line[0] == ' ');
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
