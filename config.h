#ifndef _XIYOUBU_CONFIG_H
#define _XIYOUBU_CONFIG_H

#include "enumerations.h"

namespace SETUP {
/******************************/
/*  CONSOLE LED REGION CODING */
/******************************/
constexpr ELed \
/*
    Available colours:

    BLACK, (Off)
    RED, YELLOW,
    GREEN, CYAN,
    BLUE, MAGENTA,
    WHITE

*/
/******************************/

LED_OFF           { BLACK }, \
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
BUTTON_TAPOUT     { .8e+3 }, \
STARTUP_TIME      { .1e+4 }, \
CPU_HALT_TIME     { .4e+2 };

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

MAX_MHZ           { 13.e+6 }, \
MIN_MHZ           { 7.5e+6 }, \
PAL_MHZ           { 7.600489e+6 }, \
NTSC_MHZ          { 7.670454e+6 }, \
STEP_MI           { .1e+6 }, \
STEP_MA           { .2e+6 };

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
MNU_II             { START | B }, \

/**********************/

/******************/
/*  MENU ACTIONS  */
/******************/

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
};

/*
   Advanced Config
*/

namespace ADVANCED_SETUP {
/**********************************/
/*  CONSOLE RELATIVE EEPROM DATA  */
/**********************************/
constexpr byte_t \
/*
  What location on the EEPROM of
  the MH-ATtiny88 to use, in bytes.
*/
/**********************************/

REGION_LOC        { 0 }, \
CNTRLR_LOC        { 1 }, \

/******************************/
/*  Device Pin Configurations */
/******************************/
/*
   Expressed as bits to reflect pins on the device
   and any changes made here should be noted in 
   pins_xiyoubu.h.
*/
/************************************************/
/* Data Direction Register Configuration (@_IO) */
/************************************************/
/*
   DDRA - LED
   DDRB - CLOCK
   DDRC - CONSOLE
   DDRD - CONTROLLER

*/
LED_IO            { 0b00001011 }, \
CLOCK_IO          { 0b10101100 }, \
CONSOLE_IO        { 0b00111101 }, \
CONTROLLER_IO     { 0b00000001 }, \
/**************************************************/
/*  Port Pull-Up Resistors Configuration (@_CFG)  */
/**************************************************/
/*
   PORTA - LED
   PORTB - CLOCK
   PORTC - CONSOLE
   PORTD - CONTROLLER

*/
LED_CFG           { 0b00001011 }, \
CLOCK_CFG         { 0b10100100 }, \
CONSOLE_CFG       { 0b00000011 }, \
CONTROLLER_CFG    { 0b11111110 }, \
/****************/
/*  Read Masks  */
/****************/
/*
   PINA - LED
   PINC - CONSOLE
   PIND - CONTROLLER
*/
LED_MASK          { 0b00001011 }, \
SYSTEM_MASK       { 0b00111100 }, \
SIG_MASK          { 0b00001000 };

constexpr word_t \
/*
   When the select signal from the controller is high
   the read out from the pins are shifted left by 8
   and stored in the controller read variable.

*/
PAD_MASK          { 0b1111011000000110 }, \
PAD_CLEAR         { 0b1111111111111111 };

};
#endif//_XIYOUBU_CONFIG_H
