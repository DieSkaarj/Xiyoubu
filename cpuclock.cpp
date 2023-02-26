#include "Arduino.h"

#include "config.h"
#include "pins_xiyoubu.h"
#include "cpuclock.h"

#define SQUARE_WAVE 0x2028
#define OSC_CTRL 0x2100  // SPI Mode 2
#define PHASE_OFFSET 0xC000

const double CPUClock::_min{ MIN_MHZ };
const double CPUClock::_max{ MAX_MHZ };
const double CPUClock::_step_s{ STEP_MI };
const double CPUClock::_step_l{ STEP_MA };

const double base{ 2.5e+7 };

CPUClock::CPUClock( const frequency_t t_q):
  _frequency( t_q ),
  _step( 0.f )
{
  SETUP_CLOCK( CLOCK_IO,CLOCK_CFG );
}

const uint32_t CPUClock::calculate_frequency( const double t_freq )
{
  const uint32_t hexval{ static_cast< uint32_t >( ( t_freq * pow( 2, 28 ) / base ) )};
  const uint32_t new_word{ ( ( hexval >> 14 ) & 0x3fff ) | 0x4000 };

  return ( new_word << 16 ) | ( ( hexval & 0x3fff ) | 0x4000 );
}

void CPUClock::write_ad9833( const uint16_t t_hlf_word )
{
  uint16_t data{ t_hlf_word };

  for
  ( auto i{ 0 }; i < 16; ++i )
  {
    P_CLOCK = ( data & 0x8000 ) ?
            P_CLOCK | _BV( P_SDATA ) : P_CLOCK & ~_BV( P_SDATA );
    ;;
    P_CLOCK &= ~_BV( P_SCLOCK );
    ;;
    P_CLOCK |= _BV( P_SCLOCK );
    data <<= 1; // The half word is read a bit at a time from the top (msb)
  }

  P_CLOCK &= ~_BV( P_SDATA ); // Reset AD9833 device (active low)
  ;;
}

void CPUClock::send_ad9833( const double t_freq )
{
  const uint32_t freq{ calculate_frequency( t_freq ) };
  const uint16_t lsb{ static_cast< uint16_t >( freq ) },
        msb{ static_cast< uint16_t >( freq >> 16 ) };

  P_CLOCK &= ~_BV( P_FSYNC );
  ;;
  write_ad9833( OSC_CTRL );
  write_ad9833( lsb );
  write_ad9833( msb );
  write_ad9833( PHASE_OFFSET );
  write_ad9833( SQUARE_WAVE );
  ;;
  P_CLOCK |= _BV( P_FSYNC );
}

void CPUClock::halt( const bool t_ctrl ) {
  /* Active low */ P_CLOCK = t_ctrl ? P_CLOCK & ~_BV( P_HALT ) : P_CLOCK | _BV( P_HALT );
}

void CPUClock::step( const bool t_size ) {
  _step = t_size ? _step_l : _step_s;
}

void CPUClock::reset( const double t_mhz ) {
  noInterrupts();
  halt( true );
  delay( CPU_HALT_TIME * .5 );
  send_ad9833( _frequency = t_mhz );
  halt( false );
  interrupts();
}