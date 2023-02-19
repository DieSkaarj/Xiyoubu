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
 * Init. static variables.
 */

const eLED Console::_led[4]{ LED_OFF,LED_JAP,LED_EUR,LED_USA};
eREGION Console::_region{ load_region() };

/*
 *
 * CTORS
 *
 */

uint32_t CalculateFrequency( const double );
void SerialWrite( uint16_t );
void SerialSend( const double );

Console::Console( const uint32_t t_ticks ):
  _press_counter( 0 ),
  _crystal_val_counter( 0 ),
  _chronos( t_ticks ),
  _tap_timer( 0 ),
  _is_pressed( false ),
  _can_reset( false )
{
  CPU = CPU_INIT;
  CPU_DDR =  CPU_CONF;

  CONSOLE = CONSOLE_INIT;
  CONSOLE_DDR = CONSOLE_CONF;

  LED = LED_INIT;
  LED_DDR = LED_CONF;

  PCICR = _BV( PCIE1 );
  PCMSK1 = _BV( PCINT9 );

  SerialSend( _crystal[ 0 ] );
  reconfigure( _region );
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::init()
{
  if( ( PINC>>PC1 ) & 1 )
  {
    PORTC |= _BV( PC1 );
  }
}

int Console::restart()
{
  static uint32_t debounce{ 0 };
  const uint32_t delta{ millis()-debounce };

  noInterrupts();

  _press_counter=0;

  clear_led_port();
  CONSOLE &= ~_BV( RESET );
  delayMicroseconds( 167e+4 );
  CONSOLE |= _BV( RESET );
  set_led_color( led( _region ) );

  interrupts();

  debounce=millis();
}

void Console::overclock( const bool dir )
{
  if( dir && _crystal_val_counter<sizeof(_crystal)/sizeof(double) ) ++_crystal_val_counter;
  else if( !dir && _crystal_val_counter>0 ) --_crystal_val_counter;
  else return;

  halt( true );
  delayMicroseconds( 10 );
  SerialSend( _crystal[ _crystal_val_counter ] );
  delayMicroseconds( 10 );
  halt( false );
  check_frequency();
}

void Console::check_frequency()
{
  uint32_t dtime{ _crystal[ _crystal_val_counter ] /1e+4 },
    freq_mhz{ _crystal[ _crystal_val_counter ]/1e+6 },
    ocdelta{ freq_mhz-6 };

  eLED color;

  if( freq_mhz<8 ) color=GREEN;
  else if( freq_mhz<9 ) color=YELLOW;
  else if( freq_mhz<10 ) color=RED;
  else if( freq_mhz<11 ) color=MAGENTA;
  else if( freq_mhz<12 ) color=BLUE;
  else color=WHITE;

  for
  ( int i{ 0 };i<ocdelta;++i )
  {
    clear_led_port();
    delay( dtime );
    set_led_color( color );
    delay( dtime );
  }

  delay( dtime );
  set_led_color( led( _region ) );
}

void Console::poll( const bool t_button )
{
  /*
   * on CHANGE
   *
   * Reset tap counter
   */

  const uint32_t ticks{ millis() },
   delta{ ticks-_chronos };

  if
  ( delta > ( BUTTON_TAPOUT ) )
    _press_counter = 0;

  if
  ( ( _is_pressed = !t_button ) )
    _can_reset=_press_counter++?false:true;

  _chronos = millis();
}

void Console::reconfigure( const eREGION t_region )
{
  /*
    * Condition cycle ensures that the console region is between
    * valid region codes.
    */
  if
    ( t_region < JAP ) _region = USA;
  else if
    ( t_region > USA ) _region = JAP;
  else
    _region=t_region;

  set_led_color( led( _region ) );
  set_sys_region( _region );
}

void Console::handle( const uint32_t t_ticks )
{
  const bool is_pressed{ _is_pressed };
  const uint8_t tap{ _press_counter };
  static bool is_reconfigured{ false };

  if( is_pressed )
  {
    switch
    ( tap )
    {
      case SINGLE_TAP:
      {
        if
        ( ( t_ticks-_tap_timer ) > BUTTON_RESET_TIME )
        {
          if( is_reconfigured )
          {
            reconfigure( region()++ );
            _can_reset = false;
          }
          else is_reconfigured = true;

          _tap_timer = t_ticks;
        }
      }
      break;

      case DOUBLE_TAP: save_region(); break;
      case TRIPLE_TAP: _press_counter = 0; break;
    }
  }
  else
  {
    if( _can_reset && ( ( t_ticks-_chronos )>( BUTTON_TAPOUT ) ) )
      _can_reset=restart();

    if( is_reconfigured ) is_reconfigured=!is_reconfigured;
  }
}

/*
 * 
 * Overclock stuff
 * 
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
  const uint32_t hexval{ static_cast< uint32_t >( ( t_freq * pow( 2,28 ) / base ) )};
  const uint32_t new_word{ ( ( hexval >> 14 ) &0x3fff ) |0x4000 };

  return ( new_word<<16 ) | ( ( hexval&0x3fff ) |0x4000 );
}

void SerialWrite( const uint16_t t_hlf_word )
{
  uint16_t data{ t_hlf_word };

  for
  ( auto i{ 0 };i<16;++i )
  {
    PORTB = ( data&0x8000 )?
    PORTB|_BV( SDATA ) : PORTB&~_BV( SDATA );

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
    msb{ static_cast< uint16_t >( freq>>16 ) };

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
