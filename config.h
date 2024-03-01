#ifndef _XIYOUBU_CONFIG_H
#define _XIYOUBU_CONFIG_H

#include "enumerations.h"

//#define OVERCLOCK
#define VIDEO
#define IGR
#define REMOTE_REGION

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

  BUTTON_RESET_TIME { 1.3e+3 }, \
  BUTTON_TAPOUT     { .8e+3 }, \
  STARTUP_TIME      { 1.e+3 }, \
  CPU_HALT_TIME     { .04e+3 };
  
  /********************/
  
  /**********************/
  /*  CRYSTAL FREQUENCY */
  /**********************/
  constexpr frequency_t \
  /*
     The frequency for the PAL colorburst signal is:
     4.43361875. It may need tweaked depending on 
     the device accuracy.

     Note: for NTSC the color burst frequency is:
     3.59545.

     MIN<->MAX and Step frequencies for overclocking
     the CPU. The MD/Gen varies from region but a min
     of 7.5Mhz is given.
     
     Use 'e+6' to translate Mhz into Hz.

     Base Frequency is specific to the AD9833 Module.

    Lo: 4.43316941e+6 ?
    Hi: 4.43411000e+6 ?
    
     4.43351X3 ?
           3
  */
  /******************/

  COLOR_PAL         { 4.43361875e+6 }, \
  COLOR_NTSC        { 3.579545e+6 }, \
  COLOR_CUSTOM      { 4.43358875e+6 }, \
  VIDEO_OUT         { COLOR_CUSTOM }, \

  MAX_MHZ           { 13.e+6 }, \
  MIN_MHZ           { 7.5e+6 }, \
  PAL_MHZ           { 7.600489e+6 }, \
  NTSC_MHZ          { 7.670454e+6 }, \
  STEP_MIN          { .1e+6 }, \
  STEP_MAJ          { .2e+6 }, \
  STARTUP_FREQ      { MIN_MHZ }, \
  BASE_FREQ         { 25.e+6 };
  
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
  
  MENU_I              { START | A | C }, \
  MENU_II             { START | B }, \
  
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
  
  REGION_FORWARD    { MENU_I  | LEFT }, \
  REGION_BACKWARD   { MENU_I  | RIGHT }, \
  IN_GAME_RESET     { MENU_I  | B }, \
  SAVE_REGION       { MENU_I  | UP }, \
  CHECK_FREQUENCY   { MENU_I  | DOWN }, \
  OVERCLOCK_UP_MA   { MENU_II | UP }, \
  OVERCLOCK_UP_MI   { MENU_II | RIGHT }, \
  OVERCLOCK_DOWN_MA { MENU_II | DOWN }, \
  OVERCLOCK_DOWN_MI { MENU_II | LEFT };

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
  CNTRLR_LOC        { 1 };
  
  /**********************************/
  
  /******************************/
  /*  Device Pin Configurations */
  /******************************/
  constexpr port_t \
  /*
     Expressed as bits to reflect pins on the device
     and any changes made here should be noted in
     pins_xiyoubu.h.
  
     Details of the MH-ATtiny88 pins and ports can be found at the following website:
  
     http://ww1.microchip.com/downloads/en/DeviceDoc/atmel-9157-automotive-microcontrollers-attiny88_datasheet.pdf
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
  /************************************************/
  
  LED_IO            { 0b00001011 }, \
  CLOCK_IO          { 0b10101100 }, \
  CONSOLE_IO        { 0b00111101 }, \
  CONTROLLER_IO     { 0b00000001 }, \
  
  /************************************************/
  
  /**************************************************/
  /*  Port Pull-Up Resistors Configuration (@_CFG)  */
  /**************************************************/
  /*
     PORTA - LED
     PORTB - CLOCK
     PORTC - CONSOLE
     PORTD - CONTROLLER
  
  */
  /**************************************************/
  
  LED_CFG           { 0b00001011 }, \
  CLOCK_CFG         { 0b10100100 }, \
  CONSOLE_CFG       { 0b00000011 }, \
  CONTROLLER_CFG    { 0b11111110 }, \
  
  /**************************************************/
  
  /****************/
  /*  Read Masks  */
  /****************/
  /*
     PINA - LED
     PINC - CONSOLE
     PIND - CONTROLLER
  */
  /****************/
  
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
  PAD_CLEAR         { 0b1111111111111111 }, \
  
  /****************/
  
  /**************************************/
  /*  Oscillator (AD9833) Instructions  */
  /**************************************/
  /*
    These are AD9833 specific codes, more details can be found at the follow websites:
  
    https://www.analog.com/media/en/technical-documentation/data-sheets/ad9833.pdf
    https://www.analog.com/media/en/technical-documentation/application-notes/AN-1070.pdf
  
  */
  /**************************************/

  SINE_WAVE         { 0x2000 }, \
  SQUARE_WAVE       { 0x2028 }, \
  TRIANGLE_WAVE     { 0x2002 }, \
  OSC_CTRL          { 0x2100 }, \
  PHASE_OFFSET      { 0xC000 },
  OUTPUT_WAVE       { SQUARE_WAVE };

  /**************************************/
};

#endif//_XIYOUBU_CONFIG_H
