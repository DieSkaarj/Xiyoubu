/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the LSST License Statement and
   the GNU General Public License along with this program. If not,
   see <http://www.lsstcorp.org/LegalNotices/>.


    File: main.cpp

    Desc: Entrance

    Author: David A. Cummings (David Oberlin)
    Contact: davidandrewcummings@hotmail.co.uk

*/

#include "config.h"
#include "console.h"
#include "controller.h"
#include "pins_xiyoubu.h"

using namespace SETUP;

static const Console *mega_drive;
static const Controller *pad;

ISR( V_SELECT )
{
  pad->poll( D_SELECT,D_CONTROLLER );
}

ISR( V_CONSOLE )
{
  mega_drive->poll( D_BUTTON );
}

int main()
{
  init();

  noInterrupts();

  mega_drive = new Console( millis() );
  pad = new Controller( mega_drive );

  mega_drive->on_startup( STARTUP_TIME );

  interrupts();

  while ( true )
  {
    const milliseconds_t tocks{ millis() };

    mega_drive->handle( tocks );
    pad->handle( tocks );
  }

  return EXIT_SUCCESS;
}
