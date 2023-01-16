#ifndef _ENUMERATIONS_H
#define _ENUMERATIONS_H

enum RESET_PRESS_TYPE
{
  NOT_TAPPED = 0,
  SINGLE_TAP = 1,
  DOUBLE_TAP = 2
};

enum REGION : uint8_t{ NIL = 0x00,EUR = 0x01,JAP = 0x02,USA = 0x03 };
enum LED : uint8_t{ LED_00 = 0x00,LED_01 = 0x01,LED_10 = 0x02,LED_11 = 0x03 };

#endif//_ENUMERATIONS_H