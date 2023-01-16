#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint-gcc.h>
#include <avr/eeprom.h>

#include "enumerations.h"

#define _DEBUG

struct Console
{
  volatile bool _reset,_is_pressed;

  const static uint8_t led[4];

  uint32_t _reconf_timer;
  bool _is_reconf;

  const REGION region();

  void static restart();
  void poll();
  void reconfigure(const REGION t_region);
  void handle(const uint32_t t_ticks);

  static REGION load_region();
  void save_region();

#ifdef _DEBUG
  void flash_led();
#endif//_DEBUG
  void flash_led( const LED,const int=3 );

  Console();

  private:

  static void led(const LED) const;
  static REGION _region;
  static uint8_t _press_reset_counter;

  static void annul_press_counter();
};

inline const REGION Console::region()
{
  return _region;
}

inline REGION Console::load_region()
{
  return static_cast< REGION >( eeprom_read_byte( 0 ) );
}

inline void Console::save_region()
{
  eeprom_update_byte(0,_region);
}

inline void Console::annul_press_counter()
{
  if
  ( _press_reset_counter )
    _press_reset_counter=0;
}

#endif//_CONSOLE_H
