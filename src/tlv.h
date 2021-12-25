#ifndef __TLV_H__
#define __TLV_H__

/* Helpers for working with the .tlv file format */

extern void teliva_load_definition (lua_State* L, FILE* in);
int is_special_history_key(const char* key);

#endif
