#include "console.h"
#include "Arduino.h"

#include "pins_arduino.h"

#define CONSOLE PORTC
#define CONSOLE_DDR DDRC
#define CONSOLE_CONF 0b00111101
#define CONSOLE_INIT 0b00000011
#define SYSTEM PINC2
#define SYSTEM_CLEAR ( CONSOLE &= ~(0B1111<<SYSTEM) )
#define REGION(v) ( CONSOLE |= v<<SYSTEM )
#define LED_PIN PINC4
#define LED(v) ( CONSOLE &= ~(0B11<<LED_PIN), CONSOLE |= (v<<LED_PIN) )
#define LIGHT(v) (v<<LED_PIN)
#define RESET_HOLD 800U

/*
 * Init. static variables.
 */

const uint8_t Console::led[4]
{
    LED_00,LED_01,LED_11,LED_10
};

REGION Console::_region{ static_cast< REGION >( 0 ) };
uint8_t Console::_press_reset_counter{ 0 };

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
  PCIFR = _BV(PCIF1);
  PCMSK1 = _BV(PCINT9); 

  reconfigure( load_region() );
  _press_reset_counter=0;
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::poll()
{  
  /*
   * _reset is set on CHANGE
   * _is_pressed is used to test wether it
   * was up or down.
   */
  _reset=!_reset;
  _is_pressed=_is_pressed&&~_reset? \
  false : true;
  if( _is_pressed ) _press_reset_counter++;
}

void Console::restart()
{
  CONSOLE &=~( 0B1<<PINC0 );
  /*
   * After much thought and lots of navel gazing it was determined that 42
   * was the best integer to supply the delay function with here.
   */
  delay(42U);
  /*
   * Re-engage that line!
   */
  CONSOLE |=_BV( PINC0 );
}

void Console::reconfigure(const REGION t_region)
{
  /*
    * Condition cycle ensures that the console region is between
    * valid region codes.
    */
  if
    ( t_region < EUR ) _region = USA;
  else if
    ( t_region > USA ) _region = EUR;
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
      /* Timeout */
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
      flash_led(LED_01);
      flash_led(LED_10);
      flash_led(LED_01);

      annul_press_counter();
    }
    break;
  }
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