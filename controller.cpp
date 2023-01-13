#include "Arduino.h"
#include "pins_arduino.h"

#include "controller.h"
#include "console.h"

#define CONTROLLER PORTD
#define CONTROLLER_READ PIND
#define CONTROLLER_DDR DDRD
#define CLEAR B00000000
#define SELECT PIND2
#define BUTTON_HOLD 500U

/*
 *
 * CTORS
 *
 */

Controller::Controller(Console &t_console):
  console(t_console)
{
  /*
   *  Set Port D to read and clear pull-up resistors 
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
  _on_read &= signal==false? \
  (buttons<<8)|(~0xff) : (buttons|0xff);

  _on_read |= signal==true? \
  (~buttons<<8)&(~0xff) : (buttons^0xff);
}

void Controller::handle(const uint32_t delta)
{
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
    debounce=delta;
  }

  last_read=status;

  if
  ( ( delta - debounce < BUTTON_HOLD ) )
    return;

  switch
  ( status )
  {
    case REGION_FWD:
      console.reset_system( static_cast< Console::ERegion >( console.region()+1 ) );
    break;

    case REGION_BCK:
      console.reset_system( static_cast< Console::ERegion >( console.region()-1 ) );
    break;

    case IGR:
      console.restart();
    break;
  }

  debounce=delta;
}
