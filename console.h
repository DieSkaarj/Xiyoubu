#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint-gcc.h>
#define _DEBUG

struct Console
{
  enum ERegion : uint8_t{ NIL = 0x00,EUR = 0x01,JAP = 0x02,USA = 0x03 };
  enum ELed{ LED_00 = 0x00,LED_01 = 0x01,LED_10 = 0x02,LED_11 = 0x03 };

  volatile bool _reset,_is_pressed;

  const static uint8_t led[4];

  uint32_t _reconf_timer;
  bool _is_reconf;

  const ERegion region();

  void poll();
  void restart();
  void reconfigure(const ERegion t_region,const uint32_t t_ticks);
  void handle(const uint32_t t_ticks);

#ifdef _DEBUG
  void flash_led();
#endif//_DEBUG

  Console();

  private:

  ERegion _region;

  ERegion load_region();
  void save_region();
};

#endif//_CONSOLE_H
