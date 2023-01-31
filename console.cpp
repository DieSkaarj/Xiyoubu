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
#define RESET_HOLD 800U

/*
 * Init. static variables.
 */

const uint8_t Console::led[4]{ OFF,BLUE|RED,RED,BLUE };
REGION Console::_region{ static_cast< REGION >( load_region() ) };
uint8_t Console::_press_reset_counter{ 0 };
OverClock Console::_clock;
/*
 *
 * CTORS
 *
 */

Console::Console()
{
  CONSOLE_DDR = CONSOLE_CONF;
  CONSOLE = CONSOLE_INIT;

  PCICR = _BV(PCIE1);
  PCMSK1 = _BV(PCINT8); 
  PCIFR |= _BV(PCIF1);

  reconfigure( _region );
  annul_press_counter();
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::init_clock()
{
  SPI.begin();

  _clock.halt( true );
  delay(50);
  _clock.reset();
  delay(50);
  _clock.set_frequency( OverClock::base );
  _clock.halt( false );
}

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
  _reset=!_reset;

  _is_pressed=_is_pressed&&!_reset? \
  false : true;
  if( _is_pressed ) _press_reset_counter++;
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
  const bool is_held{ _is_pressed };
  const uint8_t tap{ _press_reset_counter };
  /* No time to explain! */
  static uint32_t chronos,
                  hold_timer,
                  timeout;
  static bool has_reconf{ false };

  switch
  ( tap )
  {
    case NOT_TAPPED:
    {
      if( !is_held)
      {
        hold_timer=chronos=t_ticks;

        if( has_reconf==true ) has_reconf=false; 
      }
    }
    break;

    case SINGLE_TAP:
    {
      if
      ( is_held )
      {
        if
        ( ( hold_timer-chronos ) > RESET_HOLD )
        {
          reconfigure( static_cast< REGION >( region()+1 ) );
          has_reconf=true;
          chronos=t_ticks;
        }
        else
          hold_timer=t_ticks;

        timeout=t_ticks;
      }
      else
      if
      ( ( t_ticks-timeout ) > RESET_HOLD )
      {
        if( has_reconf==false ) restart();

        annul_press_counter();
      }
      else
      if
      ( has_reconf )
        annul_press_counter();
    }
    break;

    case DOUBLE_TAP:
    {
      save_region();
      annul_press_counter();
    }
    break;
  }
}
