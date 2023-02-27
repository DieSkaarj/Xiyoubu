#include "Arduino.h"
#include "pins_arduino.h"
#include "pins_xiyoubu.h"

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

   CTORS

*/

#define INCREASE true
#define DECREASE false
#define MAJOR true
#define MINOR false

using namespace SETUP;
using namespace ADVANCED_SETUP;

Controller::Controller( const Console *&t_console ):
  console( t_console )
{
  /*
      Set Port D to input and clear pull-up resistors
  */

  SETUP_CONTROLLER( CONTROLLER_IO,CONTROLLER_CFG );

  CONTROLLER_INTERRUPT( SELECT_SIGNAL );
  CONTROLLER_INTR_MASK( SELECT_SIGMSK );
}

/*

   FUNCTIONS

*/

void Controller::clear()
{
  _on_read = PAD_CLEAR;
}

void Controller::poll( const bool t_signal, const port_t t_buttons )
{
  /*
     The idea here is just to copy the port register into the _on_read variable.
     When the SELECT signal from the console is high the ports' values are
     shifted 8 bits to the left.

     The registers read from an active low signal e.g.
     if   ( SIGNAL == LOW  && START/C:pin1 == LOW ) then 'START' is expressed
     elif ( SIGNAL == HIGH && START/C:pin1 == LOW ) then 'C' is expressed

     To accommodate for 6 Button pads just sample from the first change.
  */
  static int sample{ 0 };

  ++sample;

  if( sample > 8 ) sample = 0;
  if( sample > 1 ) return;

  _on_read &= t_signal == false ? \
              ( t_buttons | 0xfe ) << 8 :
              ( t_buttons | 0xfe );

  _on_read |= t_signal == true ? \
              ( t_buttons ^ 0xfe ) << 8 :
              ( t_buttons ^ 0xfe );
}

void Controller::handle( const milliseconds_t t_ticks )
{
  if ( console->is_controller_available == false ) return;

  static uint16_t last_read{ 0 };
  static uint32_t delta{ BUTTON_HOLD };
  const uint16_t  status{ ( _on_read & PAD_MASK ) };
  const uint32_t debounce{ t_ticks - delta };

  if
  ( status != last_read || debounce > BUTTON_HOLD )
    delta = t_ticks;
  else
    return;

  switch
  ( status )
  {
    case OVERCLOCK_UP_MI:   console->overclock( INCREASE, MINOR ); break;
    case OVERCLOCK_UP_MA:   console->overclock( INCREASE, MAJOR ); break;
    case OVERCLOCK_DOWN_MI: console->overclock( DECREASE, MINOR ); break;
    case OVERCLOCK_DOWN_MA: console->overclock( DECREASE, MAJOR ); break;
    case REGION_FORWARD:    console->reconfigure( console->region()-1 ); break;
    case REGION_BACKWARD:   console->reconfigure( console->region()+1 ); break;
    case IN_GAME_RESET:     console->restart(); clear(); break;
    case SAVE_REGION:       console->save_region(); break;
    case CHECK_FREQUENCY:   console->check_frequency(); break;
    default: delta = t_ticks; break;
  }

  last_read = status;
}
