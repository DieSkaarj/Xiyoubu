#include "console.h"
#include <SPI.h>

#include "pins_arduino.h"

#define CONSOLE PORTC
#define CONSOLE_DDR DDRC
#define CONSOLE_CONF 0b10111110
#define CONSOLE_INIT 0b00000001
#define SYSTEM PINC2
#define SYSTEM_CLEAR ( CONSOLE &= ~(0B101111<<SYSTEM) )
#define REGION(v) ( CONSOLE |= v<<SYSTEM )
#define LED_PIN PINC4
#define LED(v) ( CONSOLE &= ~(0B1011<<LED_PIN), CONSOLE |= (v<<LED_PIN) )
#define LIGHT(v) (v<<LED_PIN)
#define RESET_HOLD 1300U

/*
 * Init. static variables.
 */

const uint8_t Console::led[4]{ OFF,BLUE|RED,RED,GREEN|RED };
REGION Console::_region{ static_cast< REGION >( load_region() ) };
OverClock Console::_clock;
/*
 *
 * CTORS
 *
 */

Console::Console( const uint32_t t_ticks ):
  _press_reset_counter( 0 ),
  _chronos( t_ticks ),_timer_a( t_ticks ),
  _timer_b( t_ticks ),_has_reconf( false ),
  _is_pressed( false )
{
  CONSOLE_DDR = CONSOLE_CONF;
  CONSOLE = CONSOLE_INIT;

  reconfigure( _region );

  PCICR = _BV(PCIE1);
  PCIFR |= _BV(PCIF1);
  PCMSK1 = _BV(PCINT8);
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::restart()
{
  CONSOLE &=~( 0B1<<PINC1 );
  /*
   * After much thought and lots of navel gazing it was determined that 42
   * was the best integer to supply the delay function with here.
   */
  delay(42U);
  /*
   * Re-engage that line!
   */
  CONSOLE |=_BV( PINC1 );
}


void Console::overclock( float amt )
{
/*  flash_led( GREEN|BLUE,10 );

  float freq{  };
  if( freq+amt > 125e+5 
  || freq+amt < 7e+6 ) return;

  halt( true );
  _clock.IncrementFrequency( REG0,amt );
  halt( false );*/
}

void Console::flash_led() const
{
  /*
   * A completely superfluous function for debugging.
   * The Xiyoubu equivalent of printf("HERE!");
   */
  for( int i{ 0 };i<9;++i)
  {
    for( int j{ 0 };j<3;++j)    
    {
      LED( 1+j );
      delay(50);
    }
  }

  LED(led[_region]);
}

void Console::flash_led( const LED t_led,const int t_time ) const
{
  for( int i{ 0 };i<t_time;++i)
  {
    LED(t_led);
    delay(50);   
  }

  LED(led[_region]);
}

void Console::poll()
{
  /*
   * _reset is set on CHANGE
   * _is_pressed is used to cut noise on up.
   */
  _reset=( PINC0&0x1 );

  _is_pressed=_is_pressed&&~_reset? \
  false : true;
  if( _is_pressed ) ++_press_reset_counter;
}

void Console::reconfigure(const REGION t_region)
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

  /*
    * Clear system bits then reset.
    */
  SYSTEM_CLEAR;
  LED(led[_region]);
  REGION(_region);
}

void Console::handle( const uint32_t t_ticks )
{
  const bool is_pressed{ _is_pressed };
  const uint8_t tap{ _press_reset_counter };

  if
  ( is_pressed )
  {
    switch
    ( tap )
    {
      case SINGLE_TAP:
      {
        if
        ( ( _timer_b-_chronos ) > RESET_HOLD )
        {
          _has_reconf = true;
          reconfigure( static_cast< REGION >( region()+1 ) );
          _chronos = t_ticks;
        }
        else
          _timer_b = t_ticks;
      }
      break;

      case DOUBLE_TAP:
      {
        _press_reset_counter = 0;
        save_region();        
      }
      break;
    }

    _timer_a = t_ticks;
  }
  else
  {
    if( ( t_ticks-_timer_a ) > ( RESET_HOLD*0.5 ) && _press_reset_counter == 1 )
    {
      _press_reset_counter = 0;
      restart();
    }

    if( _has_reconf )
    {
      _press_reset_counter = 0;
      _has_reconf=!_has_reconf;
    }

    _timer_b = _chronos = t_ticks;
  }
}
