#ifndef _CPU_CLOCK_H
#define _CPU_CLOCK_H

class CPUClock
{
  static const double _min, _max, _step_s, _step_l;
  double _frequency, _step;

  const uint32_t calculate_frequency( const frequency_t );
  void write_ad9833( const word_t );
  void send_ad9833( const frequency_t );

  public:

    void halt( const bool );
    void step( const bool );
    void reset( const frequency_t );

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
