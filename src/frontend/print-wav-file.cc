#include <iostream>

#include "alsa_devices.hh"
#include "audio_device_claim.hh"
#include "eventloop.hh"
#include "stats_printer.hh"
#include "wav_wrapper.hh"
#include <alsa/asoundlib.h>

using namespace std;

void program_body( const string_view wav_filename )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* open the WAV file */
  WavWrapper note_sample { string( wav_filename ) };
  size_t next_sample_to_print = 0;

  while ( not note_sample.at_end( next_sample_to_print ) ) {
    cout << next_sample_to_print << " " << note_sample.view( next_sample_to_print ).first << " "
         << note_sample.view( next_sample_to_print ).second << "\n";
    next_sample_to_print++;
  }
}

void usage_message( const string_view argv0 )
{
  cerr << "Usage: " << argv0 << " [wav_filename]\n";
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
