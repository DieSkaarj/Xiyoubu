#ifndef _CONTROLLER_H
#define _CONTROLLER_H

class Console;

class Controller
{
  const static uint8_t  PAD_MASK{ 0b00001001 },
                        SIG_MASK{ 0b00001000 };

  volatile uint16_t _on_read;
  Console           &console;

/*
 * HIGH bits will be stored as the most significant bits and LOW the least
 */
  enum : int16_t
  {
    START = _BV(1),
    A = _BV(2),
    UP_LO = _BV(4),
    DOWN_LO = _BV(5),
    PAD3 = (0B11 << 6),
    C = _BV(9),
    B = _BV(10),
    UP_HI = _BV(12),
    DOWN_HI = _BV(13),
    LEFT = _BV(14),
    RIGHT = _BV(15),
    PAD6 = (0B11 << 14)
  };

  const static uint16_t \
    SYSTEM_MENU{ START | A | C },
    OVERCLOCK_MENU{ START | B },
    OC_INC{ OVERCLOCK_MENU | UP_LO },
    OC_DEC{ OVERCLOCK_MENU | DOWN_LO },
    OC_CHECKUP{ OVERCLOCK_MENU | UP_LO },
    REGION_FWD{ SYSTEM_MENU | RIGHT },
    REGION_BCK{ SYSTEM_MENU | LEFT },
    REGION_SAVE{ SYSTEM_MENU | UP_HI },
    REGION_LOAD{ SYSTEM_MENU | DOWN_HI },
    IGR{ SYSTEM_MENU | B };

  public:

  void poll();
  void handle(const uint32_t);

  Controller(Console &);
};

#endif//_CONTROLLER_H
