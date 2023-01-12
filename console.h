#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint-gcc.h>

struct Console
{
  enum ERegion{ NIL = 0x00,EUR = 0x01,JAP = 0x02,USA = 0x03 };
  enum ELed{ LED_00 = 0x00,LED_01 = 0x01,LED_10 = 0x02,LED_11 = 0x03 };

  const static uint8_t led[4];

  const ERegion region();

  void console_restart();
  void reset_system(const ERegion t_region);
  void flash_led();
  void handle(const uint32_t t_delta);

  Console();

  private:

  ERegion _region;

  void save_region();
};

#endif//_CONSOLE_H
