#include <iostream>

#include "fft.hh"
#include "mmap.hh"
#include "signal.hh"
#include "wav_wrapper.hh"

using namespace std;

void program_body( const string_view wav_filename,
                   const string_view period_size_str,
                   const string_view period_number_str,
                   const string_view fft_wisdom_filename )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* open the WAV file */
  WavWrapper note_sample { string( wav_filename ) };

  const uint32_t period_size = stoi( string( period_size_str ) );
  const uint32_t period_number = stoi( string( period_number_str ) );

  TimeDomainSignal signal { period_size, 48000.0 };

  for ( size_t i = 0; i < period_size; i++ ) {
    signal.signal().at( i ) = { note_sample.view( period_number * period_size + i ).first, 0 };
  }

  FFTW fftw { false };
  ReadOnlyFile wisdom { string( fft_wisdom_filename ) };
  fftw.load_wisdom( wisdom );

  ForwardFFT transform { period_size };

  BasebandFrequencyDomainSignal fd_signal { period_size, 48000.0 };

  transform.execute( signal, fd_signal );

  for ( const auto& sample : fd_signal.signal() ) {
    cout << sample.real() << " " << sample.imag() << "\n";
  }
}

void usage_message( const string_view argv0 )
{
  cerr << "Usage: " << argv0 << " [wav_filename] [period_size] [period_number] [fft_wisdom_filename]\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort();
    }

    if ( argc != 5 ) {
      usage_message( argv[0] );
      return EXIT_FAILURE;
    }

    program_body( argv[1], argv[2], argv[3], argv[4] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
