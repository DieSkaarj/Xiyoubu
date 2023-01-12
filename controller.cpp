#include "Arduino.h"
#include "pins_arduino.h"

#include "controller.h"
#include "console.h"

#define CONTROLLER PORTD
#define CONTROLLER_READ PIND
#define CONTROLLER_DDR DDRD
#define CLEAR B00000000
#define SELECT PIND2
#define BUTTON_HOLD 5000U

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
  CONTROLLER_DDR = CLEAR;
  CONTROLLER = CLEAR;
  /*
   * Set interrupt mode to logical CHANGE on INT0 (pin18/D2.)
   */
  EICRA = 0x01;
  EIMSK = _BV(INT0);

  sei();      
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
  const bool    signal{(sig_mask&CONTROLLER_READ)>>SELECT ? true : false };
  const uint8_t buttons{pad_mask|CONTROLLER_READ};

  /*
   * The controllers' port is an 8Bit register. To only manipulate the HIGH and LOW
   * bits a 255 (0xFF) mask is applied and inverted when necessary.
   */
/*
  if( signal )
  {
      _on_read &= (buttons|0xff);
      _on_read |= (buttons<<8)&(~0xff);
  }
  else
  {
      _on_read &= (buttons<<8)&(~0xff);
      _on_read |= (buttons&0xff);
  }
*/
  _on_read &= signal==false? \
  (buttons<<8)|(~0xff) : (buttons|0xff);

  _on_read |= signal==true? \
  (~buttons<<8)&(~0xff) : (buttons^0xff);
}

void Controller::handle(const uint32_t delta)
{
  Console &md = console;
  /*
   * If lines connected to Left and Right are low the console asserts that a
   * 3BTN controller is present. These values are masked when handling,
   * as only the button presses are relevant here.
   */
  const uint16_t  status{ (_on_read&~(PAD3)) };
  static uint16_t last_read{ 0 };
  static uint32_t debounce{ 0 };

  /*
   * Reset time delta if no buttons are pressed
   */

  if
  ( (status!=last_read ) )
  {
    debounce=millis();
  }

  last_read=status;

  if
  ( ( millis() - debounce > BUTTON_HOLD ) )
    return;

  switch
  ( status )
  {
    case REGION_FWD:
    {
      md.reset_system \
      (
        static_cast<Console::ERegion> \
        (md.region()+1) \
      );
      delay(500U);
    }
    break;

    case REGION_BCK:
    {
      md.reset_system \
      (
        static_cast<Console::ERegion> \
        (md.region()-1) \
      );
      delay(500U);
    }
    break;

    case IGR:
    {
      md.console_restart();
      md.flash_led();
    }
    break;
  }
}
