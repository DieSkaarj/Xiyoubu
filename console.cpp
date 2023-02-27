#include "console.h"
#include "cpuclock.h"
#include "config.h"

#include "pins_arduino.h"
#include "pins_xiyoubu.h"

#include <avr/eeprom.h>

/*
   Init. static variables.
*/

using namespace SETUP;
using namespace ADVANCED_SETUP;

uint32_t Console::_chronos{ 0 },
         Console::_tap_timer{ 0 };
CPUClock Console::_clock( MIN_MHZ );
constexpr Console::Mode Console::mode[4];

/*

   CTORS

*/

Console::Console( const milliseconds_t t_ticks ):
  _console_region( INV ),
  _tap( 0 ),
  _is_button_pressed( false ),
  _can_reconfigure( false ),
  _is_reconfigured( false ),
  _is_overclocked( false ),
  _lock( true ),
  is_controller_available( load_controller_preference() )
{
  EXT_PIN_INTERRUPTS( ENABLE_CONSOLE );
  EXT_PIN_MASK_VECTS( V_BUTTON );

  SETUP_LED( LED_IO,LED_CFG );
  SETUP_CONSOLE( CONSOLE_IO,CONSOLE_CFG );

  _clock.reset( _clock );
}

/*

   FUNCTIONS

*/

static const ERegion Console::load_region() {
  return static_cast< ERegion >( eeprom_read_byte( REGION_LOC ) );
}

void Console::clear_sys_port() {
  P_CONSOLE &= ~SYSTEM_MASK;
}

void Console::clear_led_port() {
  P_LED &= ~LED_MASK;
}

void Console::write_led_port( const uint8_t v ) {
  P_LED |= v & LED_MASK;
}

void Console::write_sys_port( const uint8_t v ) {
  P_CONSOLE |= v & SYSTEM_MASK;
}

void Console::set_sys_region( const ERegion region ) {
  clear_sys_port();
  write_sys_port( region << 4 | region << 2 );
}

void Console::set_led_color( const ELed color )      {
  clear_led_port();
  write_led_port( color );
}

const ELed Console::led() const {
  return mode[ _console_region ].led;
}

bool Console::load_controller_preference() {
  return eeprom_read_byte( CNTRLR_LOC );
}

void Console::flip_use_controller() {
  is_controller_available = !is_controller_available;
  noInterrupts();
  eeprom_update_byte( CNTRLR_LOC, is_controller_available );
  interrupts();
  check_controller_preference();
}

void Console::default_tap() { /* Dummy Function */ }

void Console::cycle_region_timeout( const milliseconds_t t_ticks )
{
  static uint32_t timer{ 0 };

  if
  ( ( t_ticks - timer ) >= BUTTON_RESET_TIME )
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

void Console::cycle_region_reset( const milliseconds_t t_ticks )
{
  if ( _can_reconfigure ) 
  {
    if( _is_reconfigured )
    {
      _tap = _is_reconfigured = false;
    }

    _can_reconfigure = false;
  }
}

void Console::led_info( ELed t_color1, ELed t_color2 = 0 ) {

  t_color2 = t_color2 ? t_color2 : t_color1;

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

void Console::restart()
{
  noInterrupts();
  {
    clear_led_port();
    P_CONSOLE &= ~_BV( P_RESET );
    delayMicroseconds( 168e+4 );
    P_CONSOLE |= _BV( P_RESET );
    delay( 16800 );

  }
  interrupts();

  set_led_color( led() );
}

void Console::overclock( const bool dir, const bool sz )
{
  if( !_is_overclocked ) _is_overclocked = true;

  _clock.step( sz );

  if ( dir ) ++_clock;
  else --_clock;

  check_frequency();
  _clock.reset( _clock );
}

void Console::on_startup( const milliseconds_t t_wait )
{
  reconfigure();
  delay( t_wait );

  check_controller_preference();
  _lock = false;
}

void Console::check_frequency()
{
  ELed color;

  if ( _clock < 8.e+6 ) color = GREEN;
  else if ( _clock < 9.e+6 ) color = YELLOW;
  else if ( _clock < 10.e+6 ) color = RED;
  else if ( _clock < 11.e+6 ) color = MAGENTA;
  else if ( _clock < 12.e+6 ) color = BLUE;
  else color = WHITE;

  led_info( color );
}

void Console::poll( const bool t_button )
{
  /*
     on CHANGE
  */

  if ( !t_button )
  {
    _is_button_pressed = true;
  }
  else
  {
    _is_button_pressed = false;
    ++_tap;
    _chronos = millis();
  }
}

void Console::reconfigure( const ERegion t_region )
{
  /*
      Condition cycle ensures that the console region is between
      valid codes.

  */

  if
  ( t_region > USA ) _console_region = JAP;
  else if
  ( t_region < JAP ) _console_region = USA;
  else
    _console_region = t_region;
/*
  if( !_is_overclocked )
  {
    if( _console_region == USA || _console_region == JAP )
      _clock.reset( NTSC_MHZ );
    else
      _clock.reset( PAL_MHZ );
  }
*/
  set_sys_region( region() );
  set_led_color( led() );
}

void Console::tap_timeout( const milliseconds_t t_ticks, void( Console::*t_func)() )
{
  if
  ( ( t_ticks - _chronos ) >= BUTTON_TAPOUT )
  {
    _tap = 0;
    _chronos = t_ticks;
    ( this->*t_func )();
  }
}

void Console::handle( const milliseconds_t t_ticks )
{
  if ( _lock ) return;

  if( _is_button_pressed )
  {
    cycle_region_timeout( t_ticks );
  }
  else
  {
    cycle_region_reset( t_ticks );

    switch
    ( _tap )
    {
      case SINGLE_TAP: tap_timeout( t_ticks, &restart ); return;
      case DOUBLE_TAP: tap_timeout( t_ticks, &save_region ); return;
      case TRIPLE_TAP: tap_timeout( t_ticks, &flip_use_controller ); return;
    }

    if ( _tap > TRIPLE_TAP ) tap_timeout( t_ticks, &default_tap );
  }
}

void Console::check_controller_preference() {
  ELed color{ is_controller_available ? GREEN : RED };

  clear_led_port();
  delay( 250 );
  set_led_color( CYAN );
  delay( 250 );
  set_led_color( color );
  delay( 400 );
  set_led_color( led() );
}

const ERegion Console::region() const {
  return mode[ _console_region ].region;
}

void Console::save_region() {
  noInterrupts();

  eeprom_update_byte( REGION_LOC, _console_region );

  interrupts();

  led_info( WHITE );
}
