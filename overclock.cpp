#include <SPI.h>

#include "overclock.h"

OverClock::OverClock():
  freq( base )
{
  DDRB = 0b10000000;
  PORTB |= _BV( 7 );
}

void OverClock::reset()
{
  write_register(0x100);   // Write '1' to AD9833 Control register bit D8.
  delay(10);
}

void OverClock::set_frequency( int32_t frequency ) {

  int32_t FreqWord = ( frequency * pow( 2,28 ) ) / ref_freq;

  int16_t MSB = (int16_t)((FreqWord & 0xFFFC000) >> 14);    //Only lower 14 bits are used for data
  int16_t LSB = (int16_t)(FreqWord & 0x3FFF);

  //Set control bits 15 ande 14 to 0 and 1, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000;

  write_register(0x2100);
  write_register(LSB);                  // Write lower 16 bits to AD9833 registers
  write_register(MSB);                  // Write upper 16 bits to AD9833 registers.
  write_register(0xC000);               // Phase register
  write_register( WAVE::SQUARE );             // Exit & Reset to SINE, SQUARE or TRIANGLE
}

void OverClock::write_register( int dat ) {

  // Display and AD9833 use different SPI MODES so it has to be set for the AD9833 here.
  SPI.setDataMode(SPI_MODE2);

  PORTB &= ~_BV( 0 );
  delayMicroseconds(10);              // Give AD9833 time to get ready to receive data.

  SPI.transfer(highByte(dat));        // Each AD9833 register is 32 bits wide and each 16
  SPI.transfer(lowByte(dat));         // bits has to be transferred as 2 x 8-bit bytes.

  PORTB |= _BV( 0 );          //Write done. Set FSYNC high
}

void OverClock::halt( bool v )
{
  if( v ) PORTB &= ~_BV( 7 );
  else PORTB |= _BV( 7 );
}
