#include "Arduino.h"
#include "pins_arduino.h"
#include "pins_xiyoubu.h"

#include "controller.h"
#include "console.h"
#include "config.h"

#define INCREASE true
#define DECREASE false
#define MAJOR true
#define MINOR false

using namespace SETUP;
using namespace ADVANCED_SETUP;

volatile word_t Controller::_on_read{ ADVANCED_SETUP::PAD_CLEAR };

/*********************************************************************

  CLASS:    Controller
  NAME:     Controller
  DEPENDS:  const Console *&
  RETURNS:  Controller
  FUNCTION: Constructs Controller object and sets controller related
            hardware values


*********************************************************************/

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

/*********************************************************************

  CLASS:    Controller
  NAME:     clear
  DEPENDS:  void
  RETURNS:  void
  FUNCTION: Clears controller related pins on Xiyoubu hardware.


*********************************************************************/

void Controller::clear()
{
  _on_read = PAD_CLEAR;
}

/*********************************************************************

  CLASS:    Controller
  NAME:     sample
  DEPENDS:  void
  RETURNS:  const bool
  FUNCTION: Takes the first sample and cuts the latter. This way it
            only reads buttons related to 3 Buttons pads.


*********************************************************************/

const bool Controller::sample() const
{
  static int sample{ 0 };

  ++sample;

  if( sample > 8 ) sample = 0;
  if( sample > 1 ) return true;

  return false;
}

/*********************************************************************

  CLASS:    Controller
  NAME:     poll
  DEPENDS:  const Controller*&, const bool, const port_t
  RETURNS:  void
  FUNCTION: Takes the first sample and cuts the latter. This way it
            only reads buttons related to 3 Buttons pads. When the 
            SELECT signal from the console is high the ports' values
            are shifted 8 bits to the left.

            The registers read from an active low signal e.g.
            if   ( SIGNAL == LOW  && START/C:pin1 == LOW ) then 'START' is expressed
            elif ( SIGNAL == HIGH && START/C:pin1 == LOW ) then 'C' is expressed


*********************************************************************/

void Controller::poll( const Controller*& t_pad,const bool t_signal, const port_t t_buttons )
{
  const auto &joypad{ t_pad };

  if( joypad->sample() ) return;

  _on_read &= false == t_signal ? \
              ( t_buttons | 0xfe ) << 8 :
              ( t_buttons | 0xfe );

  _on_read |= true  == t_signal ? \
              ( t_buttons ^ 0xfe ) << 8 :
              ( t_buttons ^ 0xfe );
}

/*********************************************************************

  CLASS:    Controller
  NAME:     handle
  DEPENDS:  void
  RETURNS:  const milliseconds_t
  FUNCTION: Handles input and directs them to functions.


*********************************************************************/

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
#ifdef OVERCLOCK
    case OVERCLOCK_UP_MI:   console->shift_overclock( INCREASE, MINOR ); break;
    case OVERCLOCK_UP_MA:   console->shift_overclock( INCREASE, MAJOR ); break;
    case OVERCLOCK_DOWN_MI: console->shift_overclock( DECREASE, MINOR ); break;
    case OVERCLOCK_DOWN_MA: console->shift_overclock( DECREASE, MAJOR ); break;
    case CHECK_FREQUENCY:   console->check_frequency(); break;
#endif
#ifdef IGR
    case IN_GAME_RESET:     console->in_game_restart(); clear(); break;
#endif
#ifdef REMOTE_REGION
    case REGION_FORWARD:    console->reconfigure( console->region()-1 ); break;
    case REGION_BACKWARD:   console->reconfigure( console->region()+1 ); break;
    case SAVE_REGION:       console->save_region(); console->led_info( WHITE ); break;
#endif
    default: delta = t_ticks; break;
  }

  last_read = status;
}
