#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "Arduino.h"
#include <stdint-gcc.h>
#include <avr/eeprom.h>

#include "enumerations.h"
#include "overclock.h"

struct Console
{
  const static uint8_t led[4];
  volatile bool _reset,_is_pressed;

  void overclock( float amt );
  static void restart();
  static REGION load_region();

  const REGION region() const;
  void save_region() const;
  void flash_led() const;
  void flash_led( const LED,const int=3 ) const;

  void poll();
  void reconfigure(const REGION t_region);
  void handle(const uint32_t t_ticks);
  void init_clock();

  Console();

  private:

  static OverClock _clock;
  static REGION _region;
  static uint8_t _press_reset_counter;

  static void annul_press_counter();
};

inline REGION Console::load_region()
{
  return static_cast< REGION >( eeprom_read_byte( 0 ) );
}

inline const REGION Console::region() const
{
  return _region;
}

inline void Console::save_region() const
{
  eeprom_update_byte(0,_region);
  PORTC &= ~(0B1011<<PINC4);
  PORTC |= ( RED|GREEN|BLUE )<<PINC4;
  delay( 800 );
  PORTC &= ~(0B1011<<PINC4);
  PORTC |= ( led[ _region ] )<<PINC4;
}

inline void Console::annul_press_counter()
{
  if
  ( _press_reset_counter )
    _press_reset_counter=0;
}

#endif//_CONSOLE_H
