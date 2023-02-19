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

static Console * mega_drive;
static const Controller *pad;

ISR( INT1_vect )
{
  const bool select{ ( PIND>>PD3 ) &1 };
  const uint8_t buttons{ PIND };

  pad->poll( select,buttons );
}

ISR( PCINT1_vect )
{
  const bool button{ ( PINC>>PC1 ) &1 };

  mega_drive->poll( button );
}

void DestroyPtr( void *t_thing )
{
  delete t_thing;
  t_thing = nullptr;
}

int main()
{
  init();

  noInterrupts();

  mega_drive = new Console( millis() );
  pad = new Controller( *mega_drive );

  interrupts();

  while( true )
  {
    const uint32_t tocks{ millis() };

    mega_drive->handle( tocks );
    pad->handle( tocks );
  }

  DestroyPtr( pad );
  DestroyPtr( mega_drive );

  return 0;
}
