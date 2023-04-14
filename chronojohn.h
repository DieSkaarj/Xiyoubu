#ifndef _CHRONOJOHN_H
#define _CHRONOJOHN_H

#include "pins_arduino.h"
#include "types.h"

static milliseconds_t TICKS{ 0 };

namespace CHRONOJOHN
{  
  static void start_ticks()
  {
    TCCR1A = TCNT1 = TCCR1B = 0;
    TCCR1B |= _BV( CS12 ) | _BV( WGM12 );
    OCR1A = 3125;
    TIMSK1 = _BV( OCIE1A );

    TICKS = 0;
  }

  static const milliseconds_t ticks()
  {
    return TICKS;
  }
};

#endif//_CHRONOJOHN_H
