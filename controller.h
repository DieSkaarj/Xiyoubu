#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "config.h"

class Console;

struct Controller
{
    static volatile word_t  _on_read;
    const Console           *&console;

    void clear();
    bool sample();

  public:

    static void poll( const Controller*&,const bool, const port_t );
    void handle( const milliseconds_t );

    Controller( const Console *& );
};

#endif//_CONTROLLER_H
