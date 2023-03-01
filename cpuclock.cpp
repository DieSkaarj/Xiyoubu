#include "Arduino.h"

#include "config.h"
#include "pins_xiyoubu.h"
#include "cpuclock.h"

using namespace SETUP;
using namespace ADVANCED_SETUP;

const frequency_t CPUClock::_min{ MIN_MHZ };
const frequency_t CPUClock::_max{ MAX_MHZ };
const frequency_t CPUClock::_step_s{ STEP_MI };
const frequency_t CPUClock::_step_l{ STEP_MA };

const double base{ 2.5e+7 };

CPUClock::CPUClock( const frequency_t t_q ):
  _frequency( t_q ),
  _step( 0.f )
{
  SETUP_CLOCK( CLOCK_IO,CLOCK_CFG );
}

const big_word_t CPUClock::calculate_frequency( const frequency_t t_freq )
{
  const big_word_t hexval{ static_cast< uint32_t >( ( t_freq * pow( 2, 28 ) / base ) ) };
  const big_word_t new_word{ ( ( hexval >> 14 ) & 0x3fff ) | 0x4000 };

  return ( new_word << 16 ) | ( ( hexval & 0x3fff ) | 0x4000 );
}

void CPUClock::write_ad9833( const word_t t_hlf_word )
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

void CPUClock::send_ad9833( const frequency_t t_freq )
{
  const big_word_t freq{ calculate_frequency( t_freq ) };
  const word_t lsb{ static_cast< word_t >( freq ) },
        msb{ static_cast< word_t >( freq >> 16 ) };

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

void CPUClock::reset( const frequency_t t_mhz ) {
  noInterrupts();
  halt( true );
  delay( CPU_HALT_TIME * .5 );
  send_ad9833( _frequency = t_mhz );
  halt( false );
  interrupts();
}
