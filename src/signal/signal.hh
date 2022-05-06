#pragma once

#include <complex>
#include <vector>

#include <boost/align/aligned_allocator.hpp>

using Signal = std::vector<std::complex<double>, boost::alignment::aligned_allocator<std::complex<double>, 64>>;

class TimeDomainSignal
{
  Signal signal_;

  double sample_rate_;

public:
  TimeDomainSignal( const size_t sample_length, const double sample_rate )
    : signal_( sample_length )
    , sample_rate_( sample_rate )
  {}

  Signal& signal() { return signal_; }
  const Signal& signal() const { return signal_; }

  size_t size() const { return signal_.size(); }
  const std::complex<double>& at( const size_t index ) const { return signal_.at( index ); }
  std::complex<double>& at( const size_t index ) { return signal_.at( index ); }

  double sample_rate() const { return sample_rate_; }
  double duration() const { return signal_.size() / sample_rate_; }

  double index_to_time( const size_t index ) const { return index / sample_rate_; }

  double power() const;

  double correlation( const TimeDomainSignal& other ) const;

  double normalized_correlation( const TimeDomainSignal& other ) const;
};

TimeDomainSignal operator+( const TimeDomainSignal& a, const TimeDomainSignal& b );
TimeDomainSignal operator-( const TimeDomainSignal& a, const TimeDomainSignal& b );

TimeDomainSignal operator*( const TimeDomainSignal& a, const double value );
TimeDomainSignal operator/( const TimeDomainSignal& a, const double value );

class BasebandFrequencyDomainSignal
{
  Signal signal_;

  double sample_rate_;

public:
  BasebandFrequencyDomainSignal( const size_t sample_length, const double sample_rate )
    : signal_( sample_length )
    , sample_rate_( sample_rate )
  {
    if ( sample_length < 1 ) {
      throw std::runtime_error( "frequency-domain signal cannot be empty" );
    }
  }

  Signal& signal() { return signal_; }
  const Signal& signal() const { return signal_; }

  size_t size() const { return signal_.size(); }
  std::complex<double> at( const size_t index ) const { return signal_.at( index ); }
  std::complex<double>& at( const size_t index ) { return signal_.at( index ); }

  double sample_rate() const { return sample_rate_; }

  double index_to_frequency( const size_t index ) const
  {
    const bool is_negative_frequency = index >= signal().size() / 2;
    return sample_rate_ * ( double( index ) / signal().size() - is_negative_frequency );
  }

  double lowest_frequency() const { return index_to_frequency( signal().size() / 2 ); }
  double highest_frequency() const { return index_to_frequency( signal().size() / 2 - 1 ); }
  double highest_negative_frequency() const { return index_to_frequency( signal().size() - 1 ); }
  double lowest_positive_frequency() const { return index_to_frequency( 0 ); }

  void delay( const double tau );
  void normalize();
  void delay_and_normalize( const double tau );

  void verify_hermitian() const;
};

static constexpr double power_gain_to_dB( const double power_gain )
{
  return 10 * log10( power_gain );
}

static constexpr double amplitude_gain_to_dB( const double amplitude_gain )
{
  return 10 * log10( amplitude_gain * amplitude_gain );
}

static constexpr double dB_to_power_gain( const double dB )
{
  return exp10( dB / 10.0 );
}

static constexpr double dB_to_amplitude_gain( const double dB )
{
  return sqrt( dB_to_power_gain( dB ) );
}
