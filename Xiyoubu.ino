/*
 *
 *   Mo'Board underside pin               > +------+------+ <
 *          layout            Truth Table > |  +5V | GND  | < EUR = 01
 *       +-----------+             for    > +------+------+ < JAP = 10
 *       | 5 4 3 2 1 |           Jumpers  > |  ENG | JAP  | < USA = 11
 *       \  9 8 7 6  /                    > | 60Hz | 50Hz | <
 *         +-------+                      > +------+------+ <
 *
 * +-----------+-----------+-----------+-----------+-----------+-----------+-----------+
 * + 9:Start/C |  7:Signal |   6:A/B   |  4:Right  |  3:Left   |   2:Down  |   1:Up    |
 * +-----------+-----------+-----------+-----------+-----------+-----------+-----------+
 *    Note: 5:+5V, 8:GND
 *
 */

#include "console.h"
#include "controller.h"
#include "overclock.h"

static Console    *mega_drive;
static Controller *pad;

ISR(INT1_vect)
{
  pad->poll();
}

ISR(PCINT1_vect)
{
  mega_drive->poll();
}

void setup()
{
//  mega_drive.init_clock();
}

void destroy( void* t_thing )
{
  delete t_thing;
  t_thing = nullptr;
}

int main()
{
  sei();

  PCICR = _BV(PCIE1);
  PCMSK1 = _BV(PCINT8); 
  PCIFR |= _BV(PCIF1);

  init();

  mega_drive = new Console( millis() );
  pad = new Controller( *mega_drive );

  while( true )
  {
    const uint32_t timer{ millis() };

    pad->handle( timer );
    mega_drive->handle( timer );
  }

  destroy( mega_drive );
  destroy( pad );

  return 0;
}
