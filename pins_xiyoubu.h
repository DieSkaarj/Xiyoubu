#ifndef _PIN_XIYOUBU_H
#define _PIN_XIYOUBU_H

#include "Arduino.h"

/*
  Pre-fixes:

  D_ - Data (Read)
  M_ - Macro
  P_ - Pin/Port
  V_ - Vector

*/
#define EXT_PIN_INTERRUPTS( v ) ( PCICR  =  v )
#define EXT_PIN_MASK_VECTS( v ) ( PCMSK1  =  v )

/*
   LED pins
*/
#define P_LED PORTA
#define P_GREEN PA0
#define P_BLUE PA1
#define P_RED PA3

#define SETUP_LED(ddr,prt) ( P_LED = prt ); ( DDRA = ddr );

/*
   CPU/Overclocking pins
*/
#define P_CLOCK PORTB
#define P_FSYNC PB2 // Can be moved to another pin
#define P_SDATA PB3
#define P_SCLOCK PB5
#define P_HALT PB7

#define SETUP_CLOCK(ddr,prt) ( P_CLOCK = prt ); ( DDRB = ddr );

/*
   Console pins
*/
#define P_CONSOLE PORTC
#define P_RESET PC0
#define P_BUTTON PC1
#define P_VIDEO PC2
#define P_LANG PC3

#define D_CONSOLE PINC
#define D_BUTTON static_cast< bool >( D_CONSOLE & _BV( P_BUTTON ) )

#define V_CONSOLE PCINT1_vect
#define V_BUTTON _BV( PCINT9 )

#define ENABLE_CONSOLE _BV( PCIE1 )
#define SETUP_CONSOLE(ddr,prt) ( P_CONSOLE = prt ); ( DDRC = ddr );

/*
   Controller pins
*/
#define P_CONTROLLER PORTD
#define P_SELECT PD3

#define D_CONTROLLER PIND
#define D_SELECT static_cast< bool >( D_CONTROLLER & _BV( P_SELECT ) )

#define V_SELECT INT1_vect

#define SETUP_CONTROLLER(ddr,prt) ( P_CONTROLLER = prt ); ( DDRD = ddr );

#endif//_PIN_XIYOUBU_H
