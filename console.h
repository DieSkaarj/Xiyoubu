#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "Arduino.h"
#include <stdint-gcc.h>
#include <avr/eeprom.h>

#include "enumerations.h"

#define CLOCK PORTB

class Console
{
  void halt( bool );

  void clear_sys_port(){ PORTC &= ~(0xc); }
  void clear_led_port(){ PORTA &= ~(0xf); }

  void write_led_port( const uint8_t v ){ PORTA |= v&0xf; }
  void write_sys_port( const uint8_t v ){ PORTC |= v&0xc; }

  void set_sys_region( const eREGION region ) { clear_sys_port(); write_sys_port( region ); }
  void set_led_color( const eLED color )  { clear_led_port(); write_led_port( color ); }

  static eREGION _region;
  uint8_t _press_counter;
  uint32_t _chronos,_tap_timer;

  bool _is_pressed,_can_reset;

  const double _crystal[ 12 ]={ 
    7.e+6,
    7.5e+6,
    8.e+6,
    8.5e+6,
    9.e+6,
    9.5e+6,
    10e+6,
    10.5e+6,
    11.e+6,
    11.5e+6,
    12.e+6,
    12.5e+6,
  };

  int _crystal_val_counter;

  eLED led( eREGION v ){ return _led[ v>>2 ]; }

  const static uint8_t _led[4];

  public:

  void init();
  void overclock( const bool );
  void check_frequency();

  void restart();
  static eREGION load_region();

  const eREGION region() const;
  void save_region() const;

  void poll( const bool );
  void reconfigure( const eREGION t_region );
  void handle( const uint32_t t_ticks );

  Console( const uint32_t );
};

inline void Console::halt( const bool t_ctrl )
{
  /* Active low */
  CLOCK = t_ctrl?
  CLOCK & ~_BV( PINB7 ) : CLOCK | _BV( PINB7 );
}

inline eREGION Console::load_region()
{
  return static_cast< eREGION >( eeprom_read_byte( 0 ) );
}

inline const eREGION Console::region() const
{
  return _region;
}

inline void Console::save_region() const
{
  eeprom_update_byte(0,_region);

  set_led_color( WHITE );
  delay( 200 );
  set_led_color( led( _region ) );
  delay( 200 );
  set_led_color( WHITE );
  delay( 200 );
  set_led_color( led( _region ) );
}

#endif//_CONSOLE_H
