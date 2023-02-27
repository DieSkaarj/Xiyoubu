#ifndef _CPU_CLOCK_H
#define _CPU_CLOCK_H

class CPUClock
{
  static const double _min, _max, _step_s, _step_l;
  double _frequency, _step;

  const uint32_t calculate_frequency( const double );
  void write_ad9833( uint16_t );
  void send_ad9833( const double );

  public:

    void halt( const bool );
    void step( const bool );
    void reset( const double );

    operator double() const {
      return _frequency;
    }

    void operator++() {
      const double spec{ _frequency + _step };
      _frequency = spec > _max ? _frequency : spec;
    }

    void operator--() {
      const double spec{ _frequency - _step };
      _frequency = spec < _min ? _frequency : spec;
    }

    CPUClock( const frequency_t );
};

#endif//_CPU_CLOCK_H
