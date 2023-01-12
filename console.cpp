#include "console.h"
#include "Arduino.h"
#include "pins_arduino.h"

#define CONSOLE PORTC
#define CONSOLE_DDR DDRC
#define CONSOLE_CONF 0B00111101
#define SYSTEM PINC2
#define REGION(v) (v<<SYSTEM)
#define LED PINC4
#define LIGHT(v) (v<<LED)
#define CLEAR B00000000

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
  _region(EUR)
{
  CONSOLE_DDR = CONSOLE_CONF;  // Console Operators Reset/Lang/Video/LED 
  CONSOLE = CLEAR;

  reset_system( EUR );
}

/*
 *
 * FUNCTIONS
 *
 */

void Console::console_restart()
{
  flash_led();
  CONSOLE &=~(0B1<<PINC0);
  delay(10U);
  CONSOLE |=(0B1<<PINC0);
}

void Console::reset_system(const ERegion t_region)
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
}

void Console::handle(const uint32_t t_delta)
{
  const bool reset{ PINC1 };

  if( !reset )
  {
    console_restart();
  }
}

const Console::ERegion Console::region()
{
  return _region;
}

void Console::flash_led()
{
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

void Console::save_region()
{

}