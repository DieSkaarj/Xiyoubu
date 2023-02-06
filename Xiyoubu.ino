/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program. If not,
 * see <http://www.lsstcorp.org/LegalNotices/>.
 *
 *
 *  File: main.cpp
 *  
 *  Desc: Entrance
 *
 *  Author: David A. Cummings (David Oberlin)
 *  Contact: davidandrewcummings@hotmail.co.uk
 *  
 */

#include "console.h"
#include "controller.h"

#define OVERCLOCK

Console    *mega_drive;
Controller *pad;

ISR(INT1_vect)
{
  pad->poll();
}

ISR(PCINT1_vect)
{
  mega_drive->poll();
}

void destroy( void *t_thing )
{
  delete t_thing;
  t_thing = nullptr;
}

int main()
{
  sei();

  mega_drive = new Console( millis() );
  pad = new Controller( *mega_drive );

  init();

  while( true )
  {
    const uint32_t timer{ millis() };

    pad->handle( timer );
    mega_drive->handle( timer );
  }

  destroy( pad );
  destroy( mega_drive );

  return 0;
}
