#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "Arduino.h"
#include <stdint-gcc.h>
#include <avr/eeprom.h>

#include "enumerations.h"
#include "config.h"

#define CLOCK PORTB

class CPU_Clk
{
    const double _min, _max, _step_s, _step_l;
    mutable double _frequency, _step;

  public:

    CPU_Clk();

    void halt( const bool t_ctrl ) {
      /* Active low */ CLOCK = t_ctrl ? CLOCK & ~_BV( PINB7 ) : CLOCK | _BV( PINB7 );
    }
    void step( const bool t_size ) {
      _step = t_size ? _step_l : _step_s;
    }
    void reset( const double t_mhz );

    operator double() const {
      return _frequency;
    }
    void operator++() {
      const double spec{ _frequency + _step };
      _frequency = spec > _max ? _frequency : spec;
    }
    void operator--() {
      const double spec{ _frequency - _step };
      _frequency = spec < _min ? _frequency : spec;
    }
};

class Console
{
    struct Mode
    {
      ELed led;
      ERegion region;
    };

    /*
       Name:  mode
       Desc:  Lookup table for region switching.

    */
    constexpr static Mode mode[4] {
      { LED_OFF, INV },
      { LED_JAP, JAP },
      { LED_EUR, EUR },
      { LED_USA, USA }
    };

    static ERegion load_region() {
      return static_cast< ERegion >( eeprom_read_byte( 0 ) );
    }

    void reset_clock();

    void clear_sys_port() {
      PORTC &= ~(0x3c);
    }

    void clear_led_port() {
      PORTA &= ~(0xf);
    }

    void write_led_port( const uint8_t v ) {
      PORTA |= v & 0xf;
    }

    void write_sys_port( const uint8_t v ) {
      PORTC |= v & 0x3c;
    }

    void set_sys_region( const ERegion region ) {
      clear_sys_port();
      write_sys_port( region << 4 | region << 2 );
    }

    void set_led_color( const ELed color )      {
      clear_led_port();
      write_led_port( color );
    }

    const ELed led() const {
      return mode[ _console_region ].led;
    }

    void controller( const bool t_pad ) {
      _use_controller = t_pad;
    }

    bool load_controller_preference() {
      return eeprom_read_byte( 1 );
    }

    void flip_use_controller() {
      _use_controller = !_use_controller;

      eeprom_update_byte(1, _use_controller);

      check_controller_preference();
    }

    void default_tap() { /* Dummy Function */ }

    int tap_timeout( uint32_t, void(Console::*)() );

    int cycle_region_timeout( uint32_t t_ticks )
    {
      static uint32_t timer{ 0 };

      if
      ( _tap == SINGLE_TAP &&( t_ticks - timer ) > BUTTON_RESET_TIME )
      {
        if ( ( _is_reconfigured = _can_reconfigure ) )
        {
          reconfigure( _console_region + 1 );
        }
        else
          _can_reconfigure = true;
  
        timer = t_ticks;
      }
    }

    void tap_reset( const uint32_t t_ticks )
    {
      _tap = 0;
      _chronos = t_ticks;      
    }

    void cycle_region_reset()
    {
      if ( _can_reconfigure ) _can_reconfigure = false;
      if ( _is_reconfigured ) _tap = _is_reconfigured = false;
    }

    void led_info( ELed t_color1,ELed t_color2=0 ) {

      t_color2=t_color2?t_color2:t_color1;

      clear_led_port();
      delay( 250 );
      set_led_color( t_color1 );
      delay( 150 );
      clear_led_port();
      delay( 150 );
      set_led_color( t_color2 );
      delay( 150 );
      clear_led_port();
      delay( 250 );
      set_led_color( led() );
    }

    static uint32_t _chronos, _tap_timer;

    static CPU_Clk _clock;
    uint8_t \
    _console_region: 2,
                     _use_controller: 1,
                     _btn_press: 1,
                     _can_reconfigure: 1,
                     _is_reconfigured: 1,
                     _tap: 2;

  public:

    void on_startup( const uint32_t );
    void overclock( const bool /* Direction: Up=1/Down=0 */, const bool /* Step size: Big=1/Small=0 */ );
    void check_frequency();
    void restart();
    void save_region();
    void poll( const bool );
    void reconfigure( const ERegion t_region );
    void handle( const uint32_t t_ticks );

    bool controller() {
      return _use_controller;
    }

    void check_controller_preference() {
      ELed color{_use_controller ? GREEN : RED};

      clear_led_port();
      delay( 250 );
      set_led_color( CYAN );
      delay( 150 );
      set_led_color( color );
      delay( 150 );
      clear_led_port();
      delay( 150 );
      set_led_color( color );
      delay( 250 );
      set_led_color( led() );
    }


    const ERegion region() const {
      return mode[ _console_region ].region;
    }

    Console( const uint32_t );
};

inline constexpr Console::Mode Console::mode[4];
inline CPU_Clk Console::_clock;
inline void Console::save_region() {
  eeprom_update_byte(0, _console_region);

  led_info( WHITE );
}

#endif//_CONSOLE_H
