#ifndef _CONTROLLER_H
#define _CONTROLLER_H

class Console;

struct Controller
{
  const static uint8_t  SIG_MASK{ 0b00001000 };
  const static uint16_t PAD3{ 0xc0 },PAD6{ 0x30 },
                        PAD_MASK{ 0xfffff606 };
  volatile uint16_t     _on_read{ 0xff };
  Console               &console;

/*
 * HIGH bits will be stored as the most significant bits and LOW the least
 *
 */
  enum : uint16_t
  {
    START = 0x2,
    A = 0x4,
    C = 0x200,
    B = 0x400,
    UP = 0x1000,
    DOWN = 0x2000,
    LEFT = 0x4000,
    RIGHT = 0x8000,
  };

  const static uint16_t \
    SYSTEM{ START | A | C },
    SYSTEM_UP{ SYSTEM | UP },
    SYSTEM_DOWN{ SYSTEM | DOWN },
    SYSTEM_LEFT{ SYSTEM | LEFT },
    SYSTEM_RIGHT{ SYSTEM | RIGHT },
    SYSTEM_IGR{ SYSTEM | B },
    ALT{ START | B },
    ALT_A{ ALT | A },
    ALT_C{ ALT | C };

  public:

  void poll( const bool,const uint8_t );
  void handle( const uint32_t );

  Controller(Console &);
};

#endif//_CONTROLLER_H
