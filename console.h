#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "enumerations.h"
#include "config.h"

class CPUClock;

class Console
{
    struct Mode
    {
      ELed led;
      ERegion region;
    };

    /*
       Name:  mode
       Desc:  Lookup table for region switching.

    */
    static constexpr Mode mode[4] {
      { SETUP::LED_OFF, INV },
      { SETUP::LED_JAP, JAP },
      { SETUP::LED_EUR, EUR },
      { SETUP::LED_USA, USA }
    };

    static const ERegion load_region();
    void reset_clock();

    void clear_sys_port();
    void clear_led_port();

    void write_sys_port( const byte_t v );
    void write_led_port( const byte_t v );

    void set_sys_region( const ERegion /* Region Code */ );
    void set_led_color( const ELed color );

    void flip_use_controller();
    bool load_controller_preference();

    void default_tap();
    void tap_timeout( uint32_t /* Ticks */, void( Console::* )() /*Void() Member Function Pointer*/ );

    void cycle_region_timeout( uint32_t /* Ticks */ );
    void cycle_region_reset( uint32_t /* Ticks */ );

    const ELed led() const;
    void led_info( ELed t_color1, ELed t_color2 = 0 );

    static uint32_t _chronos, _tap_timer;

    static CPUClock _clock;
    uint8_t _console_region, _tap;
    bool _is_button_pressed,
         _can_reconfigure,
         _is_reconfigured,
         _is_overclocked,
         _lock;

  public:

    bool is_controller_available;

    void on_startup( const uint32_t /* Ticks */ );
    void overclock( const bool /* Direction: Up=1/Down=0 */, const bool /* Step size: Big=1/Small=0 */ );
    void check_frequency();
    void restart();
    void save_region();
    void poll( const bool /* RESET Button */ );
    void reconfigure( const ERegion t_region=load_region() );
    void handle( const uint32_t t_ticks );

    void check_controller_preference();
    const ERegion region() const;

    Console( const uint32_t );
};

#endif//_CONSOLE_H
