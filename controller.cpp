#include "Arduino.h"
#include "pins_arduino.h"

#include "controller.h"
#include "console.h"

#define CONTROLLER PORTD
#define CONTROLLER_READ PIND
#define CONTROLLER_DDR DDRD
#define CONTROLLER_CONF 0b00000000
#define CONTROLLER_INIT 0b00001000
#define SELECT PIND3
#define BUTTON_HOLD 1300U

/*
 *
 * CTORS
 *
 */

Controller::Controller(Console &t_console):
  console(t_console)
{
  /*
   *  Set Port D to input and clear pull-up resistors 
   */
  CONTROLLER_DDR = CONTROLLER_CONF;
  CONTROLLER = CONTROLLER_INIT;
  /*
   * Set interrupt mode to logical CHANGE on INT0 (pin18/D2.)
   */
  EICRA = _BV(ISC10);
  EIMSK = _BV(INT1);     
}

/*
 *
 * FUNCTIONS
 *
 */

void Controller::poll()
{
  /*
   * Is SELECT signal high or low?
   */
  const bool    signal{(SIG_MASK&CONTROLLER_READ)>>SELECT ? true : false };
  const uint8_t buttons{PAD_MASK|CONTROLLER_READ};
  /*
   * The controllers' port is an 8Bit register. To only manipulate the HIGH and LOW
   * bits a 255 (0xFF) mask is applied and inverted when necessary.
   */
  _on_read &= signal==false? \
  (buttons<<8)|(~0xff) : (buttons|0xff);

  _on_read |= signal==true? \
  (~buttons<<8)&(~0xff) : (buttons^0xff);
}

void Controller::handle(const uint32_t t_ticks)
{
 /*
   * If lines connected to Left and Right are low the console asserts that a
   * 3BTN controller is present. These values are masked when handling,
   * as only the button presses are relevant here.
   */

  static uint16_t last_read{ 0 };
  static uint32_t delta{ BUTTON_HOLD };
  const uint16_t  status{ ( _on_read&(  ~PAD3 ) ) };
  uint32_t debounce{ t_ticks-delta };

  if
  ( last_read!=status || debounce > BUTTON_HOLD)
    delta=t_ticks;
  else
    return;

  switch
  ( status )
  {
    case OC_INC:
      console.overclock( Console::step );
    break;

    case OC_DEC:
      console.overclock( -Console::step );
    break;
 
    case REGION_FWD:
      console.reconfigure( static_cast< REGION >( console.region()+1 ) );
    break;

    case REGION_BCK:
      console.reconfigure( static_cast< REGION >( console.region()-1 ) );
    break;

    case IGR:
      console.restart();
    break;

    case REGION_SAVE:
      console.save_region();
    break;

    case REGION_LOAD:
      console.reconfigure( console.load_region() );
    break;
  }

  last_read=status;
}
