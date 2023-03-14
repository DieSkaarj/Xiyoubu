#include "Arduino.h"
#include "pins_arduino.h"
#include "pins_xiyoubu.h"

#include "controller.h"
#include "console.h"
#include "config.h"

/*

   CTORS

*/

#define INCREASE true
#define DECREASE false
#define MAJOR true
#define MINOR false

using namespace SETUP;
using namespace ADVANCED_SETUP;

volatile word_t Controller::_on_read{ ADVANCED_SETUP::PAD_CLEAR };

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

bool Controller::sample()
{
  static int sample{ 0 };

  ++sample;

  if( sample > 8 ) sample = 0;
  if( sample > 1 ) return true;

  return false;
}

void Controller::poll( const Controller*& t_pad,const bool t_signal, const port_t t_buttons )
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

  auto &joypad{ t_pad };

  if( joypad->sample() ) return;

  _on_read &= false == t_signal ? \
              ( t_buttons | 0xfe ) << 8 :
              ( t_buttons | 0xfe );

  _on_read |= true  == t_signal ? \
              ( t_buttons ^ 0xfe ) << 8 :
              ( t_buttons ^ 0xfe );
}

void Controller::handle( const milliseconds_t t_ticks )
{
  if ( false == console->is_controller_available ) return;

  static pad_combo_t last_read{ PAD_CLEAR };
  static milliseconds_t delta{ BUTTON_TAPOUT };
  const pad_combo_t  status{ ( _on_read & PAD_MASK ) };
  const milliseconds_t debounce{ t_ticks - delta };

  if
  ( status != last_read || debounce > BUTTON_TAPOUT )
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
