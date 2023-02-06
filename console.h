#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "Arduino.h"
#include <stdint-gcc.h>
#include <avr/eeprom.h>

#include "enumerations.h"

#ifdef OVERCLOCK
  #include <AD9833.h>
  #define CLOCK PORTB
#endif

struct Console
{
  const static uint8_t led[4];

#ifdef OVERCLOCK
  static const float step;
  void overclock( float amt );
#endif

  static void restart();
  static REGION load_region();

  const REGION region() const;
  void save_region() const;
  void flash_led() const;
  void flash_led( const LED,const int=3 ) const;

  void poll();
  void reconfigure(const REGION t_region);
  void handle(const uint32_t t_ticks);

  Console( const uint32_t );

  private:

#ifdef OVERCLOCK
  AD9833 _clock;
  void halt( bool );
#endif

  static REGION _region;
  uint8_t _press_reset_counter;
  uint32_t _chronos,_timer_a,_timer_b;
  const float _frequency;
  bool _has_reconf,_is_pressed;
  volatile bool _reset;
};

#ifdef OVERCLOCK
inline void Console::halt( bool v )
{
  if( v ) CLOCK |= _BV( PINB0 );
  else CLOCK &= ~_BV( PINB0 );
}
#endif

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
  delay( 200 );
  PORTC |= ( RED|GREEN|BLUE )<<PINC4;
  delay( 200 );
  PORTC &= ~(0B1011<<PINC4);
  delay( 200 );
  PORTC |= ( RED|GREEN|BLUE )<<PINC4;
  delay( 200 );
  PORTC &= ~(0B1011<<PINC4);
  PORTC |= ( led[ _region ] )<<PINC4;
}

#endif//_CONSOLE_H
