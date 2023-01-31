#ifndef _OVERCLOCK_H
#define _OVERCLOCK_H

#include <stdint-gcc.h>

enum WAVE{ SINE=0x2000,TRIANGLE=0x2002,SQUARE=0x2028 };

struct OverClock
{
  static const float ref_freq{ 25000000.f };
  static const uint32_t step{ 5e5 },
    base{ 7e6 };

  uint32_t freq;

  void reset();
  void set_frequency( int32_t );
  void write_register( int );
  void halt(bool bv);

  OverClock();
};

#endif//_OVERCLOCK_H
