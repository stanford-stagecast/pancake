#include "signal.hh"

#include <iostream>
#include <numeric>

using namespace std;

void BasebandFrequencyDomainSignal::delay( const double tau )
{
  for ( size_t i = 0; i < size(); i++ ) {
    if ( i == 0 or i == size() / 2 ) { /* DC and Nyquist frequencies are purely real */
      continue;
    }

    const double f = index_to_frequency( i );
    const double delay_in_radians = -2 * M_PI * tau * f;
    const complex multiplier = exp( complex { 0.0, delay_in_radians } );

    signal_[i] *= multiplier;
  }
}

void BasebandFrequencyDomainSignal::normalize()
{
  const double normalization = 1.0 / size();
  for ( auto& x : signal_ ) {
    x *= normalization;
  }
}

void BasebandFrequencyDomainSignal::delay_and_normalize( const double tau )
{
  const double normalization = 1.0 / size();
  for ( size_t i = 0; i < size(); i++ ) {
    if ( i == 0 or i == size() / 2 ) { /* DC and Nyquist frequencies are purely real */
      signal_[i] *= normalization;
      continue;
    }

    const double f = index_to_frequency( i );
    const double delay_in_radians = -2 * M_PI * tau * f;
    const complex multiplier = exp( complex { 0.0, delay_in_radians } );

    signal_[i] *= multiplier * normalization;
  }
}

void BasebandFrequencyDomainSignal::verify_hermitian() const
{
  for ( size_t i = 0; i < size(); i++ ) {
    const auto& this_element = at( i );
    const auto& other_element = at( ( size() - i ) % size() );
    if ( abs( this_element - conj( other_element ) ) > 1e-7 ) {
      cerr << i << "/" << size() << ": " << this_element << " vs. " << ( ( size() - i ) % size() ) << ": "
           << conj( other_element ) << "\n";
      throw runtime_error( "not hermitian" );
    }
  }
}

double TimeDomainSignal::power() const
{
  return accumulate( signal().begin(), signal().end(), 0.0, []( auto x, auto y ) { return x + norm( y ); } )
         / double( size() );
}

double TimeDomainSignal::correlation( const TimeDomainSignal& other ) const
{
  if ( size() != other.size() ) {
    throw runtime_error( "correlation: size mismatch" );
  }

  if ( sample_rate() != other.sample_rate() ) {
    throw runtime_error( "correlation: sample-rate mismatch" );
  }

  complex<double> correlation = 0;

  for ( size_t i = 0; i < size(); i++ ) {
    correlation += conj( signal_[i] ) * other.signal_[i];
  }

  return correlation.real();
}

double TimeDomainSignal::normalized_correlation( const TimeDomainSignal& other ) const
{
  return correlation( other ) / sqrt( size() * size() * power() * other.power() );
}

TimeDomainSignal operator+( const TimeDomainSignal& a, const TimeDomainSignal& b )
{
  if ( a.size() != b.size() ) {
    throw runtime_error( "signal substraction: size mismatch" );
  }

  if ( a.sample_rate() != b.sample_rate() ) {
    throw runtime_error( "signal subtraction: sample-rate mismatch" );
  }

  TimeDomainSignal ret = a;
  for ( size_t i = 0; i < a.size(); i++ ) {
    ret.at( i ) += b.at( i );
  }

  return ret;
}

TimeDomainSignal operator-( const TimeDomainSignal& a, const TimeDomainSignal& b )
{
  return a + ( b * -1 );
}

TimeDomainSignal operator*( const TimeDomainSignal& a, const double value )
{
  TimeDomainSignal ret = a;
  for ( auto& x : ret.signal() ) {
    x *= value;
  }
  return ret;
}

TimeDomainSignal operator/( const TimeDomainSignal& a, const double value )
{
  return a * ( 1 / value );
}
