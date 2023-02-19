#include "Arduino.h"
#include "pins_arduino.h"

#include "controller.h"
#include "console.h"
#include "config.h"

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

  _on_read &= t_signal==false? \
  ( t_buttons|0xfe ) <<8:
  ( t_buttons|0xfe );

  _on_read |= t_signal==true? \
  ( t_buttons^0xfe ) <<8:
  ( t_buttons^0xfe );
}

void Controller::handle( const uint32_t t_ticks )
{

  static uint16_t last_read{ 0 };
  static uint32_t delta{ BUTTON_HOLD };
  const uint16_t  status{ ( _on_read&PAD_MASK ) };
  const uint32_t debounce{ t_ticks-delta };

  if
  ( last_read!=status || debounce > BUTTON_HOLD)
    delta=t_ticks;
  else
    return;

  switch
  ( status )
  {
    case OVERCLOCK_UP:    console.overclock( INCREASE ); break;
    case OVERCLOCK_DOWN:  console.overclock( DECREASE ); break;
    case REGION_FORWARD:  console.reconfigure( console.region()-- ); break;
    case REGION_BACKWARD: console.reconfigure( console.region()++ ); break;
    case IN_GAME_RESET:   console.restart(); break;
    case SAVE_REGION:     console.save_region(); break;
    case CHECK_FREQUENCY: console.check_frequency(); break;
  }

  last_read=status;
}
