/*
 *                                     +-------+
 *      \               +--------------|  USB  |--------------+               /
 *      |               |              +-------+              |               |
 * P_A2 |               | [ ] 15                        14[ ] |               | P_B7
 * P_A3 |               | [ ] 16                    SCK 13[*] | Clock (AD9833)| P_B5
 * P_A0 |               | [ ] A6                   MISO 12[*] | Pause (AD9833)| P_B4
 * P_A1 |               | [ ] A7                   MOSI 11[*] | Data  (AD9833)| P_B3   
 * P_C0 | Reset In      | [*] A0                     SS 10[*]~| FSYNC (AD9833)| P_B2
 * P_C1 | Reset Out     | [*] A1                         9[ ]~|               | P_B1
 * P_C2 | JP1/2 (JP/EN) | [*] A2                         8[ ] |               | P_B0
 * P_C3 | JP3/4 (50/60) | [*] A3                         7[*] | Pad Right     | P_D7   
 * P_C4 | @LED CR0      | [*] A4/SDA                     6[*] | Pad Left      | P_D6   
 * P_C5 | @LED CR1      | [*] A5/SCL                     5[*] | Pad Down      | P_D5
 * P_C7 | @LED CR2      | [*] 25                         4[*] | Pad Up        | P_D4
 *      |               | [ ] RST                   INT1/3[*] | Pad C/Start   | P_D3
 * P_D0 |               | [ ] 0                         5V[ ] |               |
 * P_D1 | Pad A/B       | [*] 1      VCC  MOSI GND     GND[ ] |               |
 * P_D2 | Pad (Signal)  | [*] 2/INT0 [*]  [ ]  [*]     VIN[ ] |               |
 *      |               |            [ ]  [ ]  [ ]            |               |
 *      |               |            MISO SCK  RST            |               |
 *      /               +--ATtiny88---------------------------+               \
 *                                   PWR       NEU
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

static Console    mega_drive;
static Controller pad(mega_drive);

ISR(INT0_vect)
{
  pad.poll();
}

ISR(PCINT1_vect)
{
  mega_drive.poll();
}

void setup()
{
  /* Empty */
}

void loop()
{
  const uint32_t timer{ millis() };

  mega_drive.handle( timer );
  pad.handle( timer );
}
