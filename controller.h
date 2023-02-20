#ifndef _CONTROLLER_H
#define _CONTROLLER_H

class Console;

struct Controller
{
    const static uint8_t  SIG_MASK{ 0b00001000 };
    const static uint16_t PAD3{ 0xc0 }, PAD6{ 0x30 },
          PAD_MASK{ 0xfffff606 };
    volatile uint16_t     _on_read{ 0xffffffff };
    Console               &console;

  public:

    operator int() {
      return &_on_read;
    }

    void poll( const bool, const uint8_t );
    void handle( const uint32_t );

    Controller(Console &);
};

#endif//_CONTROLLER_H
