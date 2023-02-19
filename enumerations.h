#ifndef _ENUMERATIONS_H
#define _ENUMERATIONS_H

#include "types.h"

enum RESET_PRESS_TYPE
{
  NOT_TAPPED = 0,
  SINGLE_TAP = 1,
  DOUBLE_TAP = 2,
  TRIPLE_TAP = 3
};

enum eREGION : uint8_t{ JAP = 0x4,EUR = 0x8,USA = 0xc };
inline eREGION operator++( eREGION v ){ if( v == JAP ) return EUR; if( v == EUR ) return USA; if( v == USA ) return JAP; };
inline eREGION operator--( eREGION v ){ if( v == JAP ) return USA; if( v == EUR ) return JAP; if( v == USA ) return EUR; };

enum eLED : uint8_t
{
  // RGB R=1000, G=0001, B=0010
  LED_OFF = 0b0000,
  
  GREEN = 0b0001,
  BLUE = 0b0010,
  CYAN = 0b0011,
  RED = 0b1000,
  YELLOW = 0b1001,
  MAGENTA = 0b1010,
  WHITE = 0b1011
};

enum : pad_button_t
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
