#ifndef __TELIVA_H__
#define __TELIVA_H__

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
#if __APPLE__
  TELIVA_BACKSPACE = 127,  /* delete */
#else
  TELIVA_BACKSPACE = KEY_BACKSPACE,
#endif
  TAB = 9,
  ENTER = 10,
  CTRL_K = 11,
  CTRL_L = 12,
  CTRL_Q = 17,
  CTRL_R = 18,
  CTRL_S = 19,
  CTRL_U = 21,
  CTRL_X = 24,
  ESC = 27,
};

#endif
