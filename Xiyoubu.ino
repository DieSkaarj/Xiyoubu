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
 * P_C4 | LED CR0       | [*] A4/SDA                     6[*] | Pad Left      | P_D6   
 * P_C5 | LED CR1       | [*] A5/SCL                     5[*] | Pad Down      | P_D5
 * P_C7 |               | [ ] 25                         4[*] | Pad Up        | P_D4
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
 *    Note: 5:VIN, 8:GND
 *
 */

#define SELECT PIND2
#define CONSOLE PORTC
#define CONSOLE_DDR DDRC
#define CONSOLE_CONF B00111101
#define SYSTEM PINC2
#define REGION(v) (v<<SYSTEM)
#define LED PINC4
#define LIGHT(v) (v<<LED)
#define CONTROLLER PORTD
#define CONTROLLER_READ PIND
#define CONTROLLER_DDR DDRD
#define CLEAR B00000000
#define PAD_DEBOUNCE 800U

const uint8_t pad_mask{ 0b00000101 },
              sig_mask{ 0b00000100 };

struct Console
{
  enum ERegion
  {
    NIL = 0x00,
    EUR = 0x01,
    JAP = 0x02,
    USA = 0x03
  };

  enum ELed
  {
    LED_00 = 0x00,
    LED_01 = 0x01,
    LED_10 = 0x02,
    LED_11 = 0x03
  };

  const uint8_t led[4]
  {
    {LED_00},{LED_01},
    {LED_11},{LED_10}
  };

  void reset_system(const ERegion t_region)
  {
/*
 * Condition cycle ensures that the console region is between
 * valid region codes.
 */
    if
      ( t_region < EUR ) _region = USA;
    else if
      ( t_region > USA ) _region = EUR;
    else
      _region=t_region;

/*
 * Clear system bits then reset.
 */
    CONSOLE &= ~(B1111<<SYSTEM);
    CONSOLE |= LIGHT(led[_region]);
    CONSOLE |= REGION(_region);
  }

  const ERegion region()
  {
    return _region;
  }

  void flash_led()
  {
    CONSOLE &= ~(B11<<LED);
    CONSOLE |= LIGHT(LED_11);
    delay(100);
    CONSOLE &= ~(B11<<LED);
    CONSOLE |= LIGHT(LED_10);
    delay(100);
    CONSOLE &= ~(B11<<LED);
    CONSOLE |= LIGHT(LED_01);
    delay(100);
    CONSOLE &= ~(B11<<LED);
    CONSOLE |= LIGHT(led[_region]);
  }

  Console():
    _region(EUR)
  {
    CONSOLE_DDR = CONSOLE_CONF;  // Console Operators Reset/Lang/Video/LED 
    CONSOLE = CLEAR;

    reset_system( EUR );
  }

  private:

    void save_region()
    {

    }

    ERegion _region;
};

class Controller {

  volatile uint16_t _on_read;
  const uint32_t    _debounce;
  uint32_t          _dt;

/*
 * HIGH bits will be stored as the most significant bits and LOW the least
 */
  enum : int16_t
  {
    START = _BV(1),
    A = _BV(3),
    UP_LO = _BV(4),
    DOWN_LO = _BV(5),
    PAD3 = (B11 << 6),
    C = _BV(9),
    B = _BV(11),
    UP_HI = _BV(12),
    DOWN_HI = _BV(13),
    LEFT = _BV(14),
    RIGHT = _BV(15),
    PAD6 = (B11 << 13)
  };

  const static uint16_t SYSTEM_MENU{ START | A | C },
    OVERCLOCK_MENU{ START | B },
    OC_INC{ OVERCLOCK_MENU | UP_LO },
    OC_DEC{ OVERCLOCK_MENU | DOWN_LO },
    OC_CHECKUP{ OVERCLOCK_MENU | UP_LO },
    REGION_FWD{ SYSTEM_MENU | RIGHT },
    REGION_BCK{ SYSTEM_MENU | LEFT },
    IGR{ SYSTEM_MENU | B };

  public:

  void poll()
  {
/*
 * Is SELECT signal high or low?
 */
    const bool    signal{(sig_mask&CONTROLLER_READ)>>SELECT ? true : false };
    const uint8_t buttons{pad_mask|CONTROLLER_READ};

/*
 * The controllers' port is an 8Bit register. To only manipulate the HIGH and LOW
 * bits a 255 (0xFF) mask is applied and inverted when necessary.
 */
    _on_read &= signal==false? \
    (buttons<<8)&(~0xff) : (buttons|0xff);

    _on_read |= signal==true? \
    (buttons<<8)^(~0xff) : (buttons^0xff);
  }

  void handle(Console &t_console)
  {
/*
 * If lines connected to Left and Right are low the console asserts that a
 * 3BTN controller is present. These values are masked when handling, 
 * as only the button presses are relevant here.
 */
    const uint16_t  status{ _on_read&~(PAD3) };
    static uint16_t last_read{0};

/*
 * Reset time delta if no buttons are pressed
 */
    if
    ( !status )
    {
      _dt=millis();
      last_read=status;
    }

    if
    ( ( millis() - _dt < _debounce ) || status!=last_read )
      return;

    switch( status )
    {
      case REGION_FWD:
      {
        t_console.reset_system \
        (
          static_cast<Console::ERegion> \
          (t_console.region()+1) \
        );
      }
      break;

      case REGION_BCK:
      {
        t_console.reset_system \
        (
          static_cast<Console::ERegion> \
          (t_console.region()-1) \
        );
      }
      break;
    }

    last_read=status;
    _dt=millis();
  }

  Controller():
    _debounce(PAD_DEBOUNCE){

/*
 *  Set Port D to input and clear pull-up resistors 
 */
    CONTROLLER_DDR = CLEAR;
    CONTROLLER = CLEAR;

/*
 * Set interrupt mode to logical CHANGE on INT0 (pin18/D2.)
 */
    EICRA = 0x01;
    EIMSK = _BV(INT0);

    sei();      
  }
};

static Console    mega_drive;
static Controller pad;

ISR(INT0_vect)
{
  pad.poll();
};

void setup()
{

}

void loop()
{
  pad.handle(mega_drive);
}
