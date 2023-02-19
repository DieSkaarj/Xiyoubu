#ifndef _XIYOUBU_CONFIG_H
#define _XIYOUBU_CONFIG_H

#include "enumerations.h"

  /* CONSOLE LED REGION CODING */
constexpr eLED \

  /*  
   *  Available colours:
   *  
   *  RED,
   *  YELLOW,
   *  GREEN,
   *  CYAN,
   *  BLUE,
   *  MAGENTA,
   *  WHITE
   *  
   */  

  LED_JAP           { MAGENTA },
  LED_EUR           { RED },
  LED_USA           { BLUE };

  /* CONTROLLER MENUS */
constexpr pad_button_t \

  /* 
   *  Available buttons are:
   *  
   *  START, A, B, C
   *  UP, DOWN, LEFT, RIGHT
   *  
   *  These values can be piped to create a menu.
   *
   */

  M_I               { START | A | C },
  M_II              { START | B };


  /* MENU ACTIONS */
const static pad_button_t \

  /*
   *  These actions describe the capabilities of
   *  Xiyoubu. Changing their values changes the 
   *  key combo required to activate them.
   *
   */

  OVERCLOCK_UP      { M_I  | UP },
  OVERCLOCK_DOWN    { M_I  | DOWN },
  REGION_FORWARD    { M_I  | LEFT },
  REGION_BACKWARD   { M_I  | RIGHT },
  IN_GAME_RESET     { M_I  | B },
  SAVE_REGION       { M_II | A },
  CHECK_FREQUENCY   { M_II | C };

#endif//_XIYOUBU_CONFIG_H
