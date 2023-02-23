#include "console.h"
#include "config.h"

#include "pins_arduino.h"

#define CONSOLE PORTC
#define CONSOLE_DDR DDRC

#define LED PORTA
#define LED_DDR DDRA

#define RESET PC0
#define BUTTON PC1
#define VIDEO PC2
#define LANG PC3

#define RGB_R PA3
#define RGB_G PA0
#define RGB_B PA1

#define CONSOLE_CONF 0xd
#define CONSOLE_INIT 0x3

#define CPU PORTB
#define CPU_DDR DDRB
#define CPU_INIT 0xa4
#define CPU_CONF 0xac

#define LED_CONF 0xb
#define LED_INIT 0x8

/*
   Init. static variables.
*/

uint32_t Console::_chronos{ 0 },
         Console::_tap_timer{ 0 };

/*

   CTORS

*/

uint32_t CalculateFrequency( const double );
void SerialWrite( uint16_t );
void SerialSend( const double );

CPU_Clk::CPU_Clk():
  _min( MIN_MHZ ), _max( MAX_MHZ ),
  _step_s( STEP_MINOR ),
  _step_l( STEP_MAJOR ),
  _frequency( MIN_MHZ ),
  _step( STEP_MINOR )
{
  /* Empty */
}

void CPU_Clk::reset( const double t_mhz )
{
  halt( true );
  delay( CPU_HALT_TIME * .5 );
  SerialSend( _frequency = t_mhz );
  delay( CPU_HALT_TIME * .5 );
  halt( false );
}

Console::Console( const uint32_t t_ticks ):
  _use_controller( false ),
  _btn_press( false ),
  _can_reconfigure( false ),
  _is_reconfigured( false ),
  _tap( 0 ),
  _console_region( INV )
{
  PCICR = _BV( PCIE1 );
  PCMSK1 = _BV( PCINT9 );

  LED = LED_INIT;
  LED_DDR = LED_CONF;
  CONSOLE = CONSOLE_INIT;
  CONSOLE_DDR = CONSOLE_CONF;
  CPU = CPU_INIT;
  CPU_DDR =  CPU_CONF;

  controller( load_controller_preference() );
  reconfigure( load_region() );

  SerialSend( _clock );
}

/*

   FUNCTIONS

*/

void Console::restart()
{
  noInterrupts();
  {

    clear_led_port();
    CONSOLE &= ~_BV( RESET );
    delayMicroseconds( 168e+4 );
    CONSOLE |= _BV( RESET ) | _BV( BUTTON );
    delay( 16800 );

  }
  interrupts();

  set_led_color( led() );
}

void Console::overclock( const bool dir, const bool sz )
{
  _clock.step( sz );

  if ( dir ) ++_clock;
  else --_clock;

  check_frequency();
  _clock.reset( _clock );
}

void Console::on_startup()
{
  check_controller_preference();

  tap_reset( millis() );
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

     Reset tap counter
  */
  const uint32_t ticks{ millis() };

  _chronos = ticks;
  
  if ( ( _btn_press = !t_button ) ) ++_tap;
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

  set_sys_region( region() );
  set_led_color( led() );
}

/*

   A complete and utter mess of flags. No human
   must gaze upon this nonsense.

*/

int Console::tap_timeout( uint32_t t_ticks, void( Console::*t_func)() )
{
  if
  ( ( t_ticks - _chronos ) > BUTTON_TAPOUT )
  {
    ( this->*t_func )();
    tap_reset( t_ticks );
    return 0;
  }
  else
    return 1;
}

void Console::handle( const uint32_t t_ticks )
{
  if( _btn_press )
  {
    cycle_region_timeout( t_ticks );
  }
  else
  {
    cycle_region_reset();

    switch
    ( _tap )
    {
      case SINGLE_TAP: tap_timeout( t_ticks, &restart ); break;
      case DOUBLE_TAP: tap_timeout( t_ticks, &save_region ); break;
      case TRIPLE_TAP: tap_timeout( t_ticks, &flip_use_controller ); break;
    }

    if ( _tap > TRIPLE_TAP ) tap_reset( t_ticks );
  }
}

/*

   Overclock stuff

*/

#define FSYNC PB2 // Can be moved to another pin
#define SCLOCK PB5
#define SDATA PB3
#define SQUARE_WAVE 0x2028
#define OSC_CTRL 0x2100  // SPI Mode 2
#define PHASE_OFFSET 0xC000

const double base{ 2.5e+7 };

uint32_t CalculateFrequency( const double t_freq )
{
  const uint32_t hexval{ static_cast< uint32_t >( ( t_freq * pow( 2, 28 ) / base ) )};
  const uint32_t new_word{ ( ( hexval >> 14 ) & 0x3fff ) | 0x4000 };

  return ( new_word << 16 ) | ( ( hexval & 0x3fff ) | 0x4000 );
}

void SerialWrite( const uint16_t t_hlf_word )
{
  uint16_t data{ t_hlf_word };

  for
  ( auto i{ 0 }; i < 16; ++i )
  {
    PORTB = ( data & 0x8000 ) ?
            PORTB | _BV( SDATA ) : PORTB & ~_BV( SDATA );
    ;;
    PORTB &= ~_BV( SCLOCK );
    ;;
    PORTB |= _BV( SCLOCK );
    data <<= 1; // The half word is read a bit at a time from the top (msb)
  }

  PORTB &= ~_BV( SDATA ); // Reset AD9833 device (active low)
  ;;
}

void SerialSend( const double t_freq )
{
  const uint32_t freq{ CalculateFrequency( t_freq ) };
  const uint16_t lsb{ static_cast< uint16_t >( freq ) },
        msb{ static_cast< uint16_t >( freq >> 16 ) };

  PORTB &= ~_BV( FSYNC );
  ;;
  SerialWrite( OSC_CTRL );
  SerialWrite( lsb );
  SerialWrite( msb );
  SerialWrite( PHASE_OFFSET );
  SerialWrite( SQUARE_WAVE );
  ;;
  PORTB |= _BV( FSYNC );
}
