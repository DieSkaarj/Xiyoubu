#ifndef _XIYOUBU_CONFIG_H
#define _XIYOUBU_CONFIG_H

#include "enumerations.h"

/******************************/
/*  CONSOLE LED REGION CODING */
/******************************/
constexpr ELed \
/*
    Available colours:

    LED_OFF,
    RED, YELLOW,
    GREEN, CYAN,
    BLUE, MAGENTA,
    WHITE

*/
/******************************/

LED_JAP           { MAGENTA }, \
LED_EUR           { RED }, \
LED_USA           { BLUE };


/******************************/

/********************/
/*  CONSOLE TIMINGS */
/********************/
constexpr milliseconds_t \
/*
   The amount of time (in ms) that it takes to
   cycle regions and how long to wait for
   another tap.

*/
/********************/

BUTTON_RESET_TIME { .13e+4 }, \
BUTTON_TAPOUT     { .1e+4 }, \
STARTUP_TIME      { .9e+4 }, \
CPU_HALT_TIME     { .15e+4 };

/********************/

/******************/
/*  CPU FREQUENCY */
/******************/
constexpr frequency_t \
/*
   MIN<->MAX and Step frequencies for overclocking
   the CPU. The MD/Gen varies from region but a min
   of 7.5Mhz is given.

*/
/******************/

MAX_MHZ           { 12.5e+6 }, \
MIN_MHZ           { 7.5e+6 }, \
STEP_MINOR        { .1e+6 }, \
STEP_MAJOR        { .2e+6 };

/******************/

/**********************/
/*  CONTROLLER MENUS  */
/**********************/
constexpr pad_combo_t \
/*
    Available buttons are:

    START, A, B, C
    UP, DOWN, LEFT, RIGHT

    These values can be piped to create a menu.

*/
/**********************/

MNU_I              { START | A | C }, \
MNU_II             { START | B };


/**********************/

/******************/
/*  MENU ACTIONS  */
/******************/
const static pad_combo_t \
/*
    These actions describe the capabilities of
    Xiyoubu. Changing their values changes the
    key combo required to activate them.

    MA:- Major
    MI:- Minor
*/
/******************/

REGION_FORWARD    { MNU_I  | LEFT }, \
REGION_BACKWARD   { MNU_I  | RIGHT }, \
IN_GAME_RESET     { MNU_I  | B }, \
SAVE_REGION       { MNU_I  | UP }, \
CHECK_FREQUENCY   { MNU_I  | DOWN }, \
OVERCLOCK_UP_MA   { MNU_II | UP }, \
OVERCLOCK_UP_MI   { MNU_II | RIGHT }, \
OVERCLOCK_DOWN_MA { MNU_II | DOWN }, \
OVERCLOCK_DOWN_MI { MNU_II | LEFT };


/******************/

#endif//_XIYOUBU_CONFIG_H
