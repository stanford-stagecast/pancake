#include "fft.hh"
#include "exception.hh"
#include "timer.hh"

#include <iostream>
#include <thread>

using namespace std;

FFTW::FFTW( const bool threaded )
  : threaded_( false )
{
  if ( threaded and thread::hardware_concurrency() > 1 ) {
    threaded_ = true;

    cerr << "Initializing threaded FFTW with " << thread::hardware_concurrency() << " threads... ";

    if ( not fftw_init_threads() ) {
      throw unix_error( "fftw_init_threads" );
    }

    fftw_plan_with_nthreads( thread::hardware_concurrency() );

    cerr << "done.\n";
  }
}

FFTW::~FFTW()
{
  if ( threaded_ ) {
    fftw_cleanup_threads();
  } else {
    fftw_cleanup();
  }
}

void FFTW::load_wisdom( const string_view str )
{
  struct str_and_index
  {
    string_view s;
    size_t index {};
  };

  const auto read_char = []( void* data ) -> int {
    const auto ptr = static_cast<str_and_index*>( notnull( "data", data ) );
    return ptr->s.at( ptr->index++ );
  };

  str_and_index state { str };

  if ( not fftw_import_wisdom( read_char, &state ) ) {
    throw runtime_error( "fftw_import_wisdom failed" );
  }
}

string FFTW::save_wisdom()
{
  unique_ptr<char> wisdom_string { notnull( "fftw_export_wisdom_to_string", fftw_export_wisdom_to_string() ) };
  return { wisdom_string.get() };
}

fftw_plan new_fft_plan( const unsigned int size, const int sign )
{
  Signal example_input( size );
  Signal example_output( size );

  if ( fftw_alignment_of( reinterpret_cast<double*>( example_input.data() ) ) ) {
    throw runtime_error( "internal error: input pointer not sufficiently aligned for FFTW" );
  }

  if ( fftw_alignment_of( reinterpret_cast<double*>( example_output.data() ) ) ) {
    throw runtime_error( "internal error: output pointer not sufficiently aligned for FFTW" );
  }

  cerr << "Planning " << ( ( sign == FFTW_FORWARD ) ? "FFT"s : "iFFT"s ) << " of size " << size << "... ";

  const auto start_time = Timer::timestamp_ns();
  const fftw_plan ret = notnull( "fftw_plan_dft_1d",
                                 fftw_plan_dft_1d( size,
                                                   reinterpret_cast<fftw_complex*>( example_input.data() ),
                                                   reinterpret_cast<fftw_complex*>( example_output.data() ),
                                                   sign,
                                                   FFTW_PATIENT | FFTW_PRESERVE_INPUT ) );
  const auto finish_time = Timer::timestamp_ns();
  cerr << "done (";
  Timer::pp_ns( cerr, finish_time - start_time );
  cerr << ").\n";

  double adds, muls, fmas;
  fftw_flops( ret, &adds, &muls, &fmas );

  cerr << "cost: " << fftw_cost( ret ) << " (" << adds << " adds, " << muls << " multiplies, " << fmas
       << " multiply-accumulates)\n";

  return ret;
}

fftw_plan premade_fft_plan( const unsigned int size, const int sign )
{
  Signal example_input( 1 );
  Signal example_output( 1 );

  if ( fftw_alignment_of( reinterpret_cast<double*>( example_input.data() ) ) ) {
    throw runtime_error( "internal error: input pointer not sufficiently aligned for FFTW" );
  }

  if ( fftw_alignment_of( reinterpret_cast<double*>( example_output.data() ) ) ) {
    throw runtime_error( "internal error: output pointer not sufficiently aligned for FFTW" );
  }

  const fftw_plan ret = fftw_plan_dft_1d( size,
                                          reinterpret_cast<fftw_complex*>( example_input.data() ),
                                          reinterpret_cast<fftw_complex*>( example_output.data() ),
                                          sign,
                                          FFTW_WISDOM_ONLY | FFTW_PRESERVE_INPUT );

  if ( not ret ) {
    throw runtime_error( "FFT for size " + to_string( size ) + " not found. You may need to run make-fft." );
  }

  return ret;
}

FFTPlan::FFTPlan( const size_t size, const int sign, const bool make_new_plan )
  : size_( size )
  , plan_( make_new_plan ? new_fft_plan( size, sign ) : premade_fft_plan( size, sign ) )
{}

FFTPlan::~FFTPlan()
{
  fftw_destroy_plan( plan_ );
}

void FFTPlan::execute( const Signal& input, Signal& output ) const
{
  if ( input.size() != size_ or output.size() != size_ ) {
    throw runtime_error( "size mismatch" );
  }

  if ( fftw_alignment_of( const_cast<double*>( reinterpret_cast<const double*>( input.data() ) ) ) ) {
    throw runtime_error( "input not sufficiently aligned" );
  }

  if ( fftw_alignment_of( const_cast<double*>( reinterpret_cast<const double*>( output.data() ) ) ) ) {
    throw runtime_error( "output not sufficiently aligned" );
  }

  /* okay to cast to mutable because plan was FFTW_PRESERVE_INPUT */
  fftw_execute_dft( plan_,
                    const_cast<fftw_complex*>( reinterpret_cast<const fftw_complex*>( input.data() ) ),
                    reinterpret_cast<fftw_complex*>( output.data() ) );
}

void ForwardFFT::execute( const TimeDomainSignal& input, BasebandFrequencyDomainSignal& output ) const
{
  if ( input.sample_rate() != output.sample_rate() ) {
    throw runtime_error( "sample-rate mismatch" );
  }

  plan_.execute( input.signal(), output.signal() );
}

void ReverseFFT::execute( const BasebandFrequencyDomainSignal& input, TimeDomainSignal& output ) const
{
  if ( input.sample_rate() != output.sample_rate() ) {
    throw runtime_error( "sample-rate mismatch" );
  }

  plan_.execute( input.signal(), output.signal() );
}

TimeDomainSignal delay( const TimeDomainSignal& signal, const double tau )
{
  ForwardFFT fft { signal.size() };
  ReverseFFT ifft { signal.size() };

  return delay( signal, tau, fft, ifft );
}

TimeDomainSignal delay( const TimeDomainSignal& signal,
                        const double tau,
                        const ForwardFFT& fft,
                        const ReverseFFT& ifft )
{
  for ( unsigned int i = 0; i < signal.size(); i++ ) {
    if ( abs( signal.at( i ).imag() ) > 1e-13 ) {
      throw runtime_error( "input signal had imaginary component at index " + to_string( i ) );
    }
  }

  BasebandFrequencyDomainSignal frequency_domain { signal.size(), signal.sample_rate() };
  fft.execute( signal, frequency_domain );

  frequency_domain.verify_hermitian();

  frequency_domain.delay_and_normalize( tau );

  frequency_domain.verify_hermitian();

  TimeDomainSignal ret { signal.size(), signal.sample_rate() };
  ifft.execute( frequency_domain, ret );

  for ( unsigned int i = 0; i < signal.size(); i++ ) {
    if ( abs( ret.at( i ).imag() ) > 1e-9 ) {
      throw runtime_error( "output signal had imaginary component at index " + to_string( i ) );
    }
  }

  return ret;
}
