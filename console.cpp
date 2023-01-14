#include "console.h"
#include "Arduino.h"
#include <avr/eeprom.h>

#include "pins_arduino.h"

#define _DEBUG

#define CONSOLE PORTC
#define CONSOLE_DDR DDRC
#define CONSOLE_CONF 0b00111101
#define CONSOLE_INIT 0b00000011
#define SYSTEM PINC2
#define REGION(v) (v<<SYSTEM)
#define LED PINC4
#define LIGHT(v) (v<<LED)
#define RESET_HOLD 800U

const uint8_t Console::led[4]
{
    LED_00,LED_01,LED_11,LED_10
};

/*
 *
 * CTORS
 *
 */

Console::Console():
  _reconf_timer( 0 ),
  _is_reconf( false ),
  _region(load_region())
{
  CONSOLE_DDR = CONSOLE_CONF;  // Console Operators Reset/Lang/Video/LED 
  CONSOLE = CONSOLE_INIT;

  PCICR = _BV(PCIE1);
  PCIFR = _BV(PCIF1);
  PCMSK1 = _BV(PCINT9); 

  reconfigure( _region,_reconf_timer );
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::poll()
{
  _reset=!_reset;
  _is_pressed=_is_pressed&&~_reset? \
  false : true;
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

void Console::reconfigure(const ERegion t_region,const uint32_t t_ticks)
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
  CONSOLE &= ~(0B1111<<SYSTEM);
  CONSOLE |= LIGHT(led[_region]);
  CONSOLE |= REGION(_region);

  _reconf_timer=t_ticks;
  _is_reconf=true;
}

void Console::handle( const uint32_t t_ticks )
{
  static uint32_t delta{ 0 };
  static uint8_t count{ 0 };
  const bool is_reset_depressed{ _is_pressed };
  uint32_t debounce{ t_ticks-delta };

  /*
   * Lookin' messy; might refactor later.
   */
  if( t_ticks-delta > 2000U && _is_reconf )
  {
    save_region();
    _is_reconf=false;
  }


  if
  ( count==1 && !is_reset_depressed )
  {
    restart();
  }

  if
  ( !is_reset_depressed )
  {
    if( count ) count = 0;
    return;
  }

  if
  ( debounce > RESET_HOLD )
  {
    delta=t_ticks;

    if( count )
      reconfigure( static_cast< ERegion >( region()+1 ),t_ticks );

    ++count;
  }
  else 
    return;
}

const Console::ERegion Console::region()
{
  return _region;
}

#ifdef _DEBUG

void Console::flash_led()
{
  /*
   * A completely superfluous function for debugging.
   * The Xiyoubu equivalent of printf("HERE!");
   */
  CONSOLE &= ~(0B11<<LED);
  CONSOLE |= LIGHT(LED_11);
  delay(100);
  CONSOLE &= ~(0B11<<LED);
  CONSOLE |= LIGHT(LED_10);
  delay(100);
  CONSOLE &= ~(0B11<<LED);
  CONSOLE |= LIGHT(LED_01);
  delay(100);
  CONSOLE &= ~(0B11<<LED);
  CONSOLE |= LIGHT(led[_region]);
}

#endif//DEBUG

Console::ERegion Console::load_region()
{
  ERegion region{ static_cast< ERegion >( eeprom_read_byte( 0 ) )};

  return region;
}

void Console::save_region()
{
  eeprom_update_byte(0,_region);
}