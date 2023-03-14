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

milliseconds_t \
Console::_chronos{ 0 },
        Console::_tap_timer{ 0 },
        Console::_cycle_timer{ 0 };

CPUClock \
Console::_clock( MIN_MHZ );

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

  SETUP_LED( LED_IO, LED_CFG );
  SETUP_CONSOLE( CONSOLE_IO, CONSOLE_CFG );

  reconfigure( load_region() );
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

  eeprom_update_byte( CNTRLR_LOC, is_controller_available = !is_controller_available );
}

void Console::default_tap() { /* Empty */ }

void Console::cycle_region_timeout( const milliseconds_t t_ticks )
{
  if
  ( ( t_ticks - _cycle_timer ) >= BUTTON_RESET_TIME )
  {
    if ( ( _is_reconfigured = _can_reconfigure ) )
    {
      reconfigure( region() + 1 );
      tap( RECONFIGURE );
    }
    else
      _can_reconfigure = true;

    _cycle_timer = t_ticks;
  }

}

void Console::cycle_region_reset()
{
  tap( static_cast< ETap >( _is_reconfigured = false ) );
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
  clear_led_port();
  P_CONSOLE &= ~_BV( P_RESET );
  delayMicroseconds( 168e+4 );
  P_CONSOLE |= _BV( P_RESET );
}

void Console::overclock( const bool dir, const bool sz )
{
  if ( !_is_overclocked ) _is_overclocked = true;

  _clock.step( sz );

  if ( dir ) ++_clock;
  else --_clock;

  check_frequency();
  _clock.reset( _clock );
}

void Console::on_startup( const milliseconds_t t_wait )
{
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

void Console::reset_button( const bool t_val )
{
  if
  ( !( _is_button_pressed = t_val ) )
  {
    _chronos = millis();
  }
}

ETap Console::tap( const ETap t_tap )
{
  _tap = t_tap;
}

void Console::poll( const Console*& t_console, const bool t_button )
{
  /*
     on CHANGE
  */

  auto &sega{ t_console };
  auto tap{ sega->tap() };

  if
  ( !t_button )
  {
    sega->reset_button( true );
  }
  else
  {
    sega->reset_button( false );
    sega->tap( static_cast< int >( 1 + tap ) );
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
      I'd love to keep this in however it seems
      to produce a sync error between this and the Z80
      so for the moment it remains as an idea only
  */
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

int Console::tap_timeout( const milliseconds_t t_ticks, void( Console::*t_func)() )
{
  if
  ( ( t_ticks - _chronos ) >= BUTTON_TAPOUT )
  {
    noInterrupts();
    auto reconf{ static_cast< ETap >( _can_reconfigure = false ) };

    ( this->*t_func )();
    interrupts();
    return 0;
  }
  return 1;
}

void Console::handle( const milliseconds_t t_ticks )
{
  if
  ( _is_button_pressed )
    cycle_region_timeout( t_ticks );
  else if
  ( _can_reconfigure )
  {
    switch
    ( tap() )
    {
      case SINGLE_TAP: if ( !tap_timeout( t_ticks, &restart ) ) {
          delay( BUTTON_RESET_TIME );
          set_led_color( led() );
          tap( NOT_TAPPED );
        } break;
      case DOUBLE_TAP: if ( !tap_timeout( t_ticks, &save_region ) ) {
          led_info( WHITE );
          tap( NOT_TAPPED );
        } break;
      case TRIPLE_TAP: if ( !tap_timeout( t_ticks, &flip_use_controller ) ) {
          check_frequency();
          tap( NOT_TAPPED );
        } break;
      case RESET_TAP:{
          cycle_region_reset();
        } break;
      default:_can_reconfigure=false; break;
    }
  }
}

void Console::check_controller_preference() {
  ELed color{ is_controller_available ? GREEN : RED };

  constexpr int h0{ BUTTON_TAPOUT / 4 },
            h1{ BUTTON_TAPOUT / 2 };

  clear_led_port();
  delay( h0 );
  set_led_color( CYAN );
  delay( h0 );
  set_led_color( color );
  delay( h1 );
  set_led_color( led() );
}

const ERegion Console::region() const {
  return mode[ _console_region ].region;
}

void Console::save_region() {

  eeprom_update_byte( REGION_LOC, _console_region );
}
