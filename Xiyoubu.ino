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
#include "chronojohn.h"
#include "pins_xiyoubu.h"

using namespace SETUP;

static const Console *MEGA_DRIVE{ nullptr };
static const Controller *PAD{ nullptr };

ISR( TIMER1_COMPA_vect )
{
  ++TICKS;
}

ISR( V_SELECT )
{
  Controller::poll( PAD,D_SELECT,D_CONTROLLER );
}

ISR( V_CONSOLE )
{
  Console::poll( MEGA_DRIVE,D_BUTTON );
}

int main()
{
  using namespace CHRONOJOHN;

  init();
  start_ticks();

  noInterrupts();

  MEGA_DRIVE = new Console( ticks() );
  PAD = new Controller( MEGA_DRIVE );

  MEGA_DRIVE->on_startup( STARTUP_TIME );

  interrupts();

  while ( true )
  {
    const milliseconds_t tocks{ millis() };

    MEGA_DRIVE->handle( tocks );
    PAD->handle( tocks );
  }

  return EXIT_SUCCESS;
}
