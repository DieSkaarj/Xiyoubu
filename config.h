#ifndef _XIYOUBU_CONFIG_H
#define _XIYOUBU_CONFIG_H

#include "enumerations.h"

/******************************/
/*  CONSOLE LED REGION CODING */
/******************************/
constexpr eLED \
    /*  
     *  Available colours:
     *  
     *  LED_OFF,
     *  RED, YELLOW,
     *  GREEN, CYAN,
     *  BLUE, MAGENTA,
     *  WHITE
     *  
     */  
/******************************/

  LED_JAP           { MAGENTA },
  LED_EUR           { RED },
  LED_USA           { BLUE };


/******************************/

/********************/
/*  CONSOLE TIMINGS */
/********************/
constexpr milliseconds_t \
    /*
     * The amount of time (in ms) that it takes to  
     * cycle regions and how long to wait for 
     * another tap.
     * 
     */
/********************/

  BUTTON_RESET_TIME { 1300 },
  BUTTON_TAPOUT     { BUTTON_RESET_TIME*.5 };


/********************/

/**********************/
/*  CONTROLLER MENUS  */
/**********************/
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
/**********************/

  MN_I              { START | A | C },
  MN_II             { START | B };


/**********************/

/******************/
/*  MENU ACTIONS  */
/******************/
const static pad_button_t \
    /*
     *  These actions describe the capabilities of
     *  Xiyoubu. Changing their values changes the 
     *  key combo required to activate them.
     *  
     */
/******************/

  OVERCLOCK_UP      { MN_I  | UP },
  OVERCLOCK_DOWN    { MN_I  | DOWN },
  REGION_FORWARD    { MN_I  | LEFT },
  REGION_BACKWARD   { MN_I  | RIGHT },
  IN_GAME_RESET     { MN_I  | B },
  SAVE_REGION       { MN_II | A },
  CHECK_FREQUENCY   { MN_II | C };


/******************/

#endif//_XIYOUBU_CONFIG_H
