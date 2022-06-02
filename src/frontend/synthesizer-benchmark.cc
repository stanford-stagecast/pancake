#include <iostream>

#include "synthesizer.hh"
#include "timer.hh"

using namespace std;

void program_body( const string& sample_directory )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* get ready to play an audio signal */
  constexpr unsigned int future_length = 305 * 4096; // always maintain the current version of the future 26 seconds
  ChannelPair audio_signal { future_length };

  Synthesizer synth { sample_directory };

  uint64_t ts = Timer::timestamp_ns();

  vector<uint64_t> times;
  times.reserve( 64 );

  for ( unsigned int i = 0; i < 64; i++ ) {
    synth.process_event( audio_signal,
                         0,
                         144, /* KEY_DOWN */
                         21,  /* lowest key */
                         10 /* low velocity */ );
    const uint64_t new_ts = Timer::timestamp_ns();

    times.push_back( new_ts - ts );
    ts = new_ts;
  }

  for ( const auto& time : times ) {
    Timer::pp_ns( std::cout, time );
    cout << "\n";
  }
}

void usage_message( const string_view argv0 )
{
  cerr << "Usage: " << argv0 << " [sample_directory]\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort();
    }

    if ( argc != 2 ) {
      usage_message( argv[0] );
      return EXIT_FAILURE;
    }

    program_body( argv[1] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
