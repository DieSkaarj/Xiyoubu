#include "console.h"

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

#define CONSOLE_CONF 0b00001101
#define CONSOLE_INIT 0b00000011

#define CPU PORTB
#define CPU_DDR DDRB

#define LED_CONF 0b00001011
#define LED_INIT 0b00000000

#define RESET_HOLD 1300U

/*
 * Init. static variables.
 */

const uint8_t Console::_led[4]{ LED_OFF,MAGENTA,RED,CYAN };
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
  _is_pressed( ( ( ~PIND>>PD3 ) &1 ) )
{
  CONSOLE = CONSOLE_INIT;
  CONSOLE_DDR = CONSOLE_CONF;

  LED = LED_INIT;
  LED_DDR = LED_CONF;

  PCICR = _BV( PCIE1 );
  PCMSK1 = _BV( PCINT9 );

  CPU = 0b10100100;
  CPU_DDR =  0b10101100;

  SerialSend( _crystal[ 0 ] );
  reconfigure( _region );
}

/*
 *
 * FUNCTIONS
 *
 */
bool _can_reset{false};

void Console::restart()
{
  CONSOLE &=~_BV( RESET );
  ;;
  ;;
  delay( 42 );
  ;;
  ;;
  CONSOLE |=_BV( RESET );
}

void Console::overclock( const bool dir )
{
  if( dir && _crystal_val_counter<sizeof(_crystal)/sizeof(double) ) ++_crystal_val_counter;
  else if( !dir && _crystal_val_counter>0 ) --_crystal_val_counter;
  else return;

  halt( true );
  ;;
  ;;
  ;;
  ;;
  SerialSend( _crystal[ _crystal_val_counter ] );
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
  ( delta > ( RESET_HOLD *.5 ) )
    _press_counter = _can_reset = false;
  else
    _can_reset = true;

  if
  ( ( _is_pressed = !t_button ) )
    ++_press_counter;

  _chronos = ticks;
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

  if
  ( is_pressed )
  {
    switch
    ( tap )
    {
      case SINGLE_TAP:
      {
        if
        ( ( t_ticks-_tap_timer ) > RESET_HOLD )
        {
          if( is_reconfigured ) reconfigure( region()++ );
          else is_reconfigured = true;

          _tap_timer = t_ticks;
        }
      }
      break;

      case DOUBLE_TAP:
      {
        save_region();
      }
      break;
    }
  }
  else /* is_pressed == false */
  {    
    if
    ( _can_reset && tap == 1
    && ( t_ticks - _chronos )>( RESET_HOLD *.5 ) )
    {
      _can_reset = false;
      restart();
    }

    if
    ( is_reconfigured )
      is_reconfigured = false;
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
