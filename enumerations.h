#ifndef _ENUMERATIONS_H
#define _ENUMERATIONS_H

#include "types.h"

enum ETap
{
  NOT_TAPPED = 0,
  SINGLE_TAP = 1,
  DOUBLE_TAP = 2,
  TRIPLE_TAP = 3
};

enum ERegion : uint8_t { INV = 0x0, JAP = 0b01, EUR = 0b10, USA = 0b11 };
inline ERegion operator++( const ERegion v ) {
  return ( ++v ) > USA ? JAP : v;
};
inline ERegion operator--( const ERegion v ) {
  return ( --v ) < JAP ? USA : v;
};

enum ELed : uint8_t
{
  /*
     PORTA PIN:
     RGB R=0b1000, G=0b0001, B=0b0010
  */
  BLACK = 0b0000,
  GREEN = 0b0001,
  BLUE = 0b0010,
  CYAN = 0b0011,
  RED = 0b1000,
  YELLOW = 0b1001,
  MAGENTA = 0b1010,
  WHITE = 0b1011
};

enum EPad : pad_combo_t
{
  START = 0x2,
  A = 0x4,
  C = 0x200,
  B = 0x400,
  UP = 0x1000,
  DOWN = 0x2000,
  LEFT = 0x4000,
  RIGHT = 0x8000,
};

#endif//_ENUMERATIONS_H
