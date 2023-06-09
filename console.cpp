#include "console.h"
#include "cpuclock.h"
#include "chronojohn.h"
#include "config.h"

#include "pins_arduino.h"
#include "pins_xiyoubu.h"

#include <avr/eeprom.h>

using namespace SETUP;
using namespace ADVANCED_SETUP;
using namespace CHRONOJOHN;

/*

   Static declarations.


*/

milliseconds_t \
Console::_chronos{ 0 }, \
Console::_tap_timer{ 0 }, \
Console::_cycle_timer{ 0 };

CPUClock \
Console::_clock( MIN_MHZ );

constexpr Console::Mode Console::mode[4];

/*********************************************************************

  CLASS:    Console
  NAME:     Console
  DEPENDS:  milliseconds_t
  RETURNS:  Console
  FUNCTION: Constructs Console object and sets console related
            hardware values


*********************************************************************/

Console::Console( const milliseconds_t t_ticks ):
  _console_region( INV ),
  _tap( 0 ),
  _is_button_pressed( false ),
  _can_reconfigure( false ),
  _can_tap( false ),
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

/*********************************************************************

  CLASS:    Console
  NAME:     load_region
  DEPENDS:  void
  RETURNS:  ERegion
  FUNCTION: Return value from hardwares' EEPROM


*********************************************************************/

static const ERegion Console::load_region() {
  return static_cast< ERegion >( eeprom_read_byte( REGION_LOC ) );
}

/*********************************************************************

  CLASS:    Console
  NAME:     clear_sys_port
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to FALSE/0


*********************************************************************/

void Console::clear_sys_port() {
  P_CONSOLE &= ~SYSTEM_MASK;
}

/*********************************************************************

  CLASS:    Console
  NAME:     clear_led_port
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to FALSE/0


*********************************************************************/

void Console::clear_led_port() {
  P_LED &= ~LED_MASK;
}

/*********************************************************************

  CLASS:    Console
  NAME:     write_sys_port
  DEPENDS:  byte_t
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to user defined values


*********************************************************************/

void Console::write_sys_port( const byte_t v ) {
  P_CONSOLE |= v & SYSTEM_MASK;
}

/*********************************************************************

  CLASS:    Console
  NAME:     write_led_port
  DEPENDS:  byte_t
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to user defined values


*********************************************************************/

void Console::write_led_port( const byte_t v ) {
  P_LED |= v & LED_MASK;
}

/*********************************************************************

  CLASS:    Console
  NAME:     set_sys_region
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to user defined values


*********************************************************************/

void Console::set_sys_region( const ERegion region ) {
  clear_sys_port();
  write_sys_port( region << 4 | region << 2 );
}

/*********************************************************************

  CLASS:    Console
  NAME:     set_led_region
  DEPENDS:  ELed
  RETURNS:  void
  FUNCTION: Reset pin values on hardware to user defined values


*********************************************************************/

void Console::set_led_color( const ELed color )      {
  clear_led_port();
  write_led_port( color );
}

/*********************************************************************

  CLASS:    Console
  NAME:     flip_use_controller
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Flips and stores a boolean value into the hardware EEPROM


*********************************************************************/

void Console::flip_use_controller() {
  eeprom_update_byte( CNTRLR_LOC, is_controller_available = !is_controller_available );
}

/*********************************************************************

  CLASS:    Console
  NAME:     load_controller_preference
  DEPENDS:  void
  RETURNS:  bool
  FUNCTION: Fetches a boolean value stored in hardware EEPROM


*********************************************************************/

bool Console::load_controller_preference() {
  return eeprom_read_byte( CNTRLR_LOC );
}

/*********************************************************************

  CLASS:    Console
  NAME:     on_tap_timeout
  DEPENDS:  milliseconds_t,void( ::*t_func)()
  RETURNS:  int
  FUNCTION: Timer that peforms a function, resets tap count
            and configure flag if test against time is true.


*********************************************************************/

int Console::on_tap_timeout( const milliseconds_t t_ticks, void( Console::*t_func)() )
{
  if
  ( ( t_ticks - _chronos ) >= BUTTON_TAPOUT )
  {
    noInterrupts();
    tap( NOT_TAPPED );
    _can_tap = false;
    ( this->*t_func )();
    _can_reconfigure = false;
    _can_tap = true;
    interrupts();
    return 0;
  }
  return 1;
}

/*********************************************************************

  CLASS:    Console
  NAME:     on_tap_timeout
  DEPENDS:  milliseconds_t
  RETURNS:  void
  FUNCTION: Timer that tests against a reset-time and determines
            whether the console will scroll through region
            configurations.


*********************************************************************/

void Console::cycle_region_timeout( const milliseconds_t t_ticks )
{
  if
  ( ( t_ticks - _cycle_timer ) >= BUTTON_RESET_TIME )
  {
    if
    ( _can_reconfigure )
    {
      reconfigure( region() + 1 );
      tap( RECONFIGURE );
    }
    else
      _can_reconfigure = true;

    _cycle_timer = t_ticks;
  }
}

/*********************************************************************

  CLASS:    Console
  NAME:     cycle_region_reset
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Reset values that determine wheteher the console can
            reconfigure.


*********************************************************************/

void Console::cycle_region_reset()
{
  noInterrupts();
  if ( tap() ) tap( NOT_TAPPED );
  if ( _can_reconfigure )  _can_reconfigure = false;
  interrupts();
}

/*********************************************************************

  CLASS:    Console
  NAME:     restart
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Soft restarts console.


*********************************************************************/

void Console::restart()
{
  clear_led_port();
  P_CONSOLE &= ~_BV( P_RESET );
  delayMicroseconds( 168e+4 );
  P_CONSOLE |= _BV( P_RESET );
}

/*********************************************************************

  CLASS:    Console
  NAME:     tap
  DEPENDS:  ETap
  RETURNS:  ETap
  FUNCTION: Reset tap member to user defined value.


*********************************************************************/

ETap Console::tap( const ETap t_tap )
{
  _tap = t_tap;
}

/*********************************************************************

  CLASS:    Console
  NAME:     led
  DEPENDS:  void
  RETURNS:  ELed
  FUNCTION: Returns ELed colour value associated with current region.


*********************************************************************/

const ELed Console::led() const {
  return mode[ _console_region ].led;
}

/*********************************************************************

  CLASS:    Console
  NAME:     on_startup
  DEPENDS:  milliseconds_t
  RETURNS:  void
  FUNCTION: Wait a pre-determined amount of time before unlocking
            reset button and informing user about controller input.


*********************************************************************/

void Console::on_startup( const milliseconds_t t_wait )
{
  delay( t_wait );

  check_controller_preference();
  _can_tap = true;
  _lock = false;
}

/*********************************************************************

  CLASS:    Console
  NAME:     shift_overclock
  DEPENDS:  bool,bool
  RETURNS:  void
  FUNCTION: Shift oscillator frequency up/down in major/minor
            increments


*********************************************************************/

void Console::shift_overclock( const bool dir, const bool sz )
{
  if ( !_is_overclocked ) _is_overclocked = true;

  _clock.step( sz );

  if ( dir ) ++_clock;
  else --_clock;

  check_frequency();
  _clock.reset( _clock );
}

/*********************************************************************

  CLASS:    Console
  NAME:     check_frequency
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Determines frequency range and flashes console LED the
            corresponding value.


*********************************************************************/

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

/*********************************************************************

  CLASS:    Console
  NAME:     in_game_restart
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Soft restarts console with user feedback.


*********************************************************************/

void Console::in_game_restart()
{
  restart();
  delay( BUTTON_RESET_TIME );
  set_led_color( led() );
}

/*********************************************************************

  CLASS:    Console
  NAME:     save_region
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Saves consoles' current region to hardware EEPROM.


*********************************************************************/

void Console::save_region() {

  eeprom_update_byte( REGION_LOC, _console_region );
}

/*********************************************************************

  CLASS:    Console
  NAME:     save_region
  DEPENDS:  Console*&,bool
  RETURNS:  void
  FUNCTION: Read hardware values and updates either time or tap
            accordingly.


*********************************************************************/

void Console::poll( const Console*& t_console, const bool t_button )
{
  /*
     on CHANGE
  */

  const auto &sega{ t_console };
  const auto tap{ sega->tap() };
  const auto can_tap{ sega->_can_tap };

  if
  ( !t_button )
  {
    sega->reset_button( true );
    _chronos = millis();
  }
  else
  {
    sega->reset_button( false );
    if( can_tap ) sega->tap( SINGLE_TAP + tap );
  }
}

/*********************************************************************

  CLASS:    Console
  NAME:     reconfigure
  DEPENDS:  ERegion
  RETURNS:  void
  FUNCTION: Reconfigures hardware pins according to users' desired
            region code.


*********************************************************************/

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

/*********************************************************************

  CLASS:    Console
  NAME:     handle
  DEPENDS:  milliseconds_t
  RETURNS:  void
  FUNCTION: Handler for console button presses.


*********************************************************************/

void Console::handle( const milliseconds_t t_ticks )
{
  if
  ( _is_button_pressed ) cycle_region_timeout( t_ticks );
  else if
  ( _can_reconfigure )
  {
    switch
    ( tap() )
    {
      case SINGLE_TAP:
      {
        if
        ( !on_tap_timeout( t_ticks, &restart ) )
        {
          delay( BUTTON_RESET_TIME );
          set_led_color( led() );
        }
      }
      break;

      case DOUBLE_TAP:
        if
        ( !on_tap_timeout( t_ticks, &save_region ) ) led_info( WHITE );
      break;

      case TRIPLE_TAP:
        if
        ( !on_tap_timeout( t_ticks, &flip_use_controller ) ) check_frequency();
      break;

      case RESET_TAP:
      {
        can_reconfigure( false );
        _chronos = t_ticks;
        tap( NOT_TAPPED );
      }
      break;
    }
  }
}

/*********************************************************************

  CLASS:    Console
  NAME:     led_info
  DEPENDS:  ELed,Eled
  RETURNS:  void
  FUNCTION: Used for feedback.


*********************************************************************/

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

/*********************************************************************

  CLASS:    Console
  NAME:     check_controller_preference
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Used for feedback on controllers' input role.


*********************************************************************/

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

/*********************************************************************

  CLASS:    Console
  NAME:     led_info
  DEPENDS:  void
  RETURNS:  ERegion
  FUNCTION: Returns current user defined console region.


*********************************************************************/

const ERegion Console::region() const {
  return mode[ _console_region ].region;
}
