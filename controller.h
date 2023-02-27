#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "config.h"

class Console;

struct Controller
{
    volatile uint16_t     _on_read{ ADVANCED_SETUP::PAD_CLEAR };
    const Console         *&console;

    void clear();

  public:

    void poll( const bool, const port_t );
    void handle( const milliseconds_t );

    Controller( const Console *& );
};

#endif//_CONTROLLER_H
