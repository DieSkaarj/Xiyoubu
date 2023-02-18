#include "Arduino.h"
#include "pins_arduino.h"

#include "controller.h"
#include "console.h"

#define CONTROLLER PORTD
#define CONTROLLER_READ PIND
#define CONTROLLER_DDR DDRD
#define CONTROLLER_CONF 0x1
#define CONTROLLER_INIT 0xfe
#define SELECT PD3
#define BUTTON_HOLD 1300U

/*
 *
 * CTORS
 *
 */

#define INCREASE true
#define DECREASE false

Controller::Controller(Console &t_console):
  console(t_console)
{
  /*
   *  Set Port D to input and clear pull-up resistors 
   */

  CONTROLLER = CONTROLLER_INIT;
  CONTROLLER_DDR = CONTROLLER_CONF;

  EICRA &= ~_BV( ISC11 );
  EICRA = _BV( ISC10 ) ;
  EIMSK = _BV( INT1 );

}

/*
 *
 * FUNCTIONS
 *
 */

void Controller::poll( const bool t_signal,const uint8_t t_buttons )
{
  /*
   * The idea here is just to copy the port register into the _on_read variable.
   * When the SELECT signal from the console is high the ports' values are
   * shifted 8 bits to the left.
   * 
   * The registers read from an active low signal e.g.
   * if   ( SIGNAL == LOW  && START/C:pin1 == LOW ) then 'START' is expressed
   * elif ( SIGNAL == HIGH && START/C:pin1 == LOW ) then 'C' is expressed 
   * 
   */

  /*
   * The controllers' port is an 8Bit register. To only manipulate the HIGH and LOW
   * bits a 255 (0xFF) mask is applied and inverted when necessary.
   */

  _on_read &= t_signal==false? \
  ( t_buttons|0xfe ) <<8:
  ( t_buttons|0xfe );

  _on_read |= t_signal==true? \
  ( t_buttons^0xfe ) <<8:
  ( t_buttons^0xfe );
}

void Controller::handle( const uint32_t t_ticks )
{
 /*
   * If lines connected to Left and Right are low the console asserts that a
   * 3BTN controller is present. These values are masked when handling,
   * as only the button presses are relevant here.
   * &~0x1c9
   */

  static uint16_t last_read{ 0 };
  static uint32_t delta{ BUTTON_HOLD };
  uint16_t  status{ ( _on_read&PAD_MASK ) };
  uint32_t debounce{ t_ticks-delta };

  if
  ( last_read!=status || debounce > BUTTON_HOLD)
    delta=t_ticks;
  else
    return;

  switch
  ( status )
  {
    case SYSTEM_UP:   console.overclock( INCREASE ); break;
    case SYSTEM_DOWN: console.overclock( DECREASE ); break;
    case SYSTEM_LEFT: console.reconfigure( console.region()-- ); break;
    case SYSTEM_RIGHT:console.reconfigure( console.region()++ ); break;
    case SYSTEM_IGR:  console.restart(); break;
    case ALT_A:       console.save_region(); break;
    case ALT_C:       console.check_frequency(); break;
  }

  last_read=status;
}
