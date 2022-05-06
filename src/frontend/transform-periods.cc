#include <iostream>

#include "alsa_devices.hh"
#include "audio_device_claim.hh"
#include "eventloop.hh"
#include "stats_printer.hh"
#include "wav_wrapper.hh"
#include <alsa/asoundlib.h>

using namespace std;

void program_body( const string_view device_prefix, const string_view wav_filename )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* create event loop */
  auto event_loop = make_shared<EventLoop>();

  /* find the audio device */
  auto [name, interface_name] = ALSADevices::find_device( { device_prefix } );

  /* claim exclusive access to the audio device */
  const auto device_claim = AudioDeviceClaim::try_claim( name );

  /* use ALSA to initialize and configure audio device */
  const auto short_name = device_prefix.substr( 0, 16 );
  auto playback_interface = make_shared<AudioInterface>( interface_name, short_name, SND_PCM_STREAM_PLAYBACK );
  AudioInterface::Configuration config;
  config.sample_rate = 48000;    /* samples per second */
  config.buffer_size = 96 * 8;   /* maximum samples of queued audio = 2 milliseconds */
  config.period_size = 16 * 8;   /* chunk size for kernel's management of audio buffer */
  config.avail_minimum = 64 * 8; /* device is writeable with 64 samples can be written */
  playback_interface->set_config( config );
  playback_interface->initialize();

  constexpr uint32_t sample_distance_to_write = 48;

  /* open the WAV file */
  WavWrapper note_sample { string( wav_filename ) };

  /* get ready to play an audio signal */
  ChannelPair audio_signal { 16384 }; // the output signal
  size_t next_sample_to_play = 0;     // what's the next sample # to be written to the output signal?

  constexpr uint32_t period_duration = 3600; // not exactly correct but close enough...

  uint32_t period_to_play = 0;

  /* open standard input for reading */
  FileDescriptor standard_input { STDIN_FILENO };

  /* rule #1: write samples from WAV file */
  event_loop->add_rule(
    "write samples",
    [&] {
      while ( ( next_sample_to_play <= playback_interface->cursor() + sample_distance_to_write ) ) {
        /* play the wav file */
        bool play_backwards = ( next_sample_to_play / period_duration ) % 2;
        // bool play_backwards = false;
        uint32_t sample_in_period = next_sample_to_play % period_duration; // ranges from 0 .. 109
        if ( play_backwards ) {
          sample_in_period = period_duration - sample_in_period; // now it ranges from 109 back to 0
        }

        sample_in_period += period_to_play * period_duration;

        audio_signal.safe_set( next_sample_to_play, note_sample.view( sample_in_period ) );
        next_sample_to_play++;
      }
    },
    /* when should this rule run? commit to an output signal until x milliseconds in the future */
    [&] { return ( next_sample_to_play <= playback_interface->cursor() + sample_distance_to_write ); } );

  /* rule #2: play the output signal whenever space available in audio output buffer */
  event_loop->add_rule(
    "play sine wave",
    playback_interface->fd(), /* file descriptor event cares about */
    Direction::Out,           /* execute rule when file descriptor is "writeable"
                                 -> there's room in the output buffer (config.buffer_size) */
    [&] {
      playback_interface->play( next_sample_to_play, audio_signal );
      /* now that we've played these samples, pop them from the outgoing audio signal */
      audio_signal.pop_before( playback_interface->cursor() );
    },
    [&] {
      return next_sample_to_play > playback_interface->cursor();
    },     /* rule should run as long as any new samples available to play */
    [] {}, /* no callback if EOF or closed */
    [&] {  /* on error such as buffer overrun/underrun, recover the ALSA interface */
          playback_interface->recover();
          return true;
    } );

  /* rule #3: advance period when input on keyboard */
  string throwaway_buffer = "hello";
  event_loop->add_rule( "advance period", standard_input, Direction::In, [&] {
    standard_input.read( string_span::from_view( throwaway_buffer ) );
    period_to_play++;
    cerr << "Advancing period to play, now " << period_to_play << "\n";
  } );

  /* run the event loop forever */
  while ( event_loop->wait_next_event( -1 ) != EventLoop::Result::Exit ) {
  }
}

void usage_message( const string_view argv0 )
{
  cerr << "Usage: " << argv0 << " [device_prefix] [wav_filename]\n";

  cerr << "Available devices:";

  const auto devices = ALSADevices::list();

  if ( devices.empty() ) {
    cerr << " none\n";
  } else {
    cerr << "\n";
    for ( const auto& dev : devices ) {
      for ( const auto& interface : dev.interfaces ) {
        cerr << "  '" << interface.second << "'\n";
      }
    }
  }
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort();
    }

    if ( argc != 3 ) {
      usage_message( argv[0] );
      return EXIT_FAILURE;
    }

    program_body( argv[1], argv[2] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
