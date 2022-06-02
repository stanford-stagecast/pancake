#include <iostream>
#include <set>

#include "eventloop.hh"
#include "midi_processor.hh"
#include "stats_printer.hh"
#include "synthesizer.hh"
#include "wav_wrapper.hh"
#include <alsa/asoundlib.h>

using namespace std;

void program_body( const string& sample_directory )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* create event loop */
  auto event_loop = make_shared<EventLoop>();

  /* get ready to play an audio signal */
  constexpr unsigned int future_length = 305 * 4096; // always maintain the current version of the future 26 seconds
  ChannelPair audio_signal { future_length };

  Synthesizer synth { sample_directory };
  MidiProcessor midi_processor {};

  /* rule #2: let synthesizer read in new MIDI processor data */
  event_loop->add_rule(
    "synthesize event",
    [&] {
      while ( midi_processor.has_event() ) {
        synth.process_event( audio_signal,
                             0,
                             midi_processor.get_event_type(),
                             midi_processor.get_event_note(),
                             midi_processor.get_event_velocity() );
        midi_processor.pop_event();
      }
    },
    /* when should this rule run? */
    [&] { return midi_processor.has_event(); } );

  /* add a task that prints statistics occasionally */
  StatsPrinterTask stats_printer { event_loop };

  /* run the event loop forever */
  while ( event_loop->wait_next_event( stats_printer.wait_time_ms() ) != EventLoop::Result::Exit ) {
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
