#include <alsa/asoundlib.h>
#include <iostream>
#include <chrono>
#include <deque>

#include "alsa_devices.hh"
#include "audio_device_claim.hh"
#include "eventloop.hh"
#include "midi_processor.hh"
#include "stats_printer.hh"
#include "simplenn.hh"

using namespace std;
using namespace chrono;

static constexpr unsigned int audio_horizon = 16; /* samples */
static constexpr float max_amplitude = 0.9;
static constexpr float note_decay_rate = 0.9998;

static string nn_file = "/usr/local/share/predict-timestamps.dnn";

array<float, 16> calculate_input( deque<float> times, int num_notes, steady_clock::time_point base_time )
{
  float curr_time_secs = duration_cast<milliseconds>( steady_clock::now() - base_time ).count()/1000.0;

  array<float, 16> ret_mat;
  for ( auto i = 0; i < 16; i++ ) {
    if ( i >= num_notes ) {
      ret_mat[i] = 0;
    } else {
      ret_mat[i] = curr_time_secs - times[i];
    }
  }
  return ret_mat;
}

void program_body( const string_view audio_device, const string& midi_device )
{
  /* speed up C++ I/O by decoupling from C standard I/O */
  ios::sync_with_stdio( false );

  /* create event loop */
  auto event_loop = make_shared<EventLoop>();

  /* find the audio device */
  auto [name, interface_name] = ALSADevices::find_device( { audio_device } );

  /* claim exclusive access to the audio device */
  const auto device_claim = AudioDeviceClaim::try_claim( name );

  /* use ALSA to initialize and configure audio device */
  const auto short_name = audio_device.substr( 0, 16 );
  auto playback_interface = make_shared<AudioInterface>( interface_name, short_name, SND_PCM_STREAM_PLAYBACK );
  AudioInterface::Configuration config;
  config.sample_rate = 48000;           /* samples per second */
  config.buffer_size = 48;              /* maximum samples of queued audio = 1 millisecond */
  config.period_size = 16;              /* chunk size for kernel's management of audio buffer */
  config.avail_minimum = audio_horizon; /* device is writeable when full horizon can be written */
  playback_interface->set_config( config );
  playback_interface->initialize();

  /* open the MIDI device */
  FileDescriptor piano { CheckSystemCall( midi_device, open( midi_device.c_str(), O_RDONLY ) ) };
  MidiProcessor midi;
  deque<float> press_queue {};
  size_t num_notes = 0;

  /* get ready to play an audio signal */
  ChannelPair audio_signal { 16384 };  // the output signal
  size_t next_sample_to_calculate = 0; // what's the next sample # to be written to the output signal?

  /* current amplitude of the sine waves */
  float amp_left = 0, amp_right = 0;

  steady_clock::time_point next_note_pred { steady_clock::now() };
  steady_clock::time_point last_pred_time { steady_clock::now() };
  steady_clock::time_point curr_time { steady_clock::now() };
  SimpleNN nn { nn_file };
  float time_since_pred_note = 0;
  array<float, 16> past_timestamps {};

  /* rule #1: write a continuous sine wave (but no more than 1.3 ms into the future) */
  event_loop->add_rule(
    "calculate sine wave",
    [&] {
      while ( next_sample_to_calculate <= playback_interface->cursor() + audio_horizon ) {
        const double time = next_sample_to_calculate / double( config.sample_rate );
        /* compute the sine wave amplitude (middle A, 440 Hz) */
        audio_signal.safe_set(
          next_sample_to_calculate,
          { amp_left * sin( 2 * M_PI * 440 * time ), amp_right * sin( 2 * M_PI * 440 * time ) } );
        amp_left *= note_decay_rate;
        // amp_right = some equation based on note_decay_rate and next_note_pred
        curr_time = steady_clock::now();
        if (next_note_pred >= curr_time) {
          time_since_pred_note = config.sample_rate/1000 * duration_cast<milliseconds>(curr_time - next_note_pred).count();
          amp_right = pow(note_decay_rate, time_since_pred_note);
        }
        next_sample_to_calculate++;
      }
    },
    /* when should this rule run? commit to an output signal until some horizon in the future */
    [&] { return next_sample_to_calculate <= playback_interface->cursor() + audio_horizon; } );

  /* rule #2: play the output signal whenever space available in audio output buffer */
  event_loop->add_rule(
    "play sine wave",
    playback_interface->fd(), /* file descriptor event cares about */
    Direction::Out,           /* execute rule when file descriptor is "writeable"
                                 -> there's room in the output buffer (config.buffer_size) */
    [&] {
      playback_interface->play( next_sample_to_calculate, audio_signal );
      /* now that we've played these samples, pop them from the outgoing audio signal */
      audio_signal.pop_before( playback_interface->cursor() );
    },
    [&] {
      return next_sample_to_calculate > playback_interface->cursor();
    },     /* rule should run as long as any new samples available to play */
    [] {}, /* no callback if EOF or closed */
    [&] {  /* on error such as buffer overrun/underrun, recover the ALSA interface */
          playback_interface->recover();
          return true;
    } );

  /* rule #3: read MIDI data */
  event_loop->add_rule(
    "read MIDI event",
    piano,
    Direction::In,
    [&] { midi.read_from_fd( piano ); },
    [&] { return not midi.data_timeout(); } );

  /* rule #4: process MIDI data */
  event_loop->add_rule(
    "process MIDI event",
    [&] {
      while ( midi.has_event() ) {
        float time_val = midi.pop_event() / 1.0;
        if ( midi.get_event_type() == 144 ) { /* key down */
          amp_left = max_amplitude;
          if ( num_notes < 16 ) {
            press_queue.push_back( time_val );
            cout << std::fixed << "time val: " << time_val << "\n";
            num_notes++;
          } else {
            press_queue.push_back( time_val );
            press_queue.pop_front();
          }
        }
        midi.pop_event();
      }
    },
    [&] { return midi.has_event(); } );

  /* rule #5: get DNN prediction */
  event_loop->add_rule(
    "get DNN prediction",
    [&] {
      last_pred_time = steady_clock::now();
      past_timestamps = calculate_input( press_queue, num_notes, last_pred_time );
      float time_to_next = nn.predict_next_timestamp( past_timestamps );
      next_note_pred = last_pred_time + round<milliseconds>(duration<float>{time_to_next});
    },
    [&] { return duration_cast<milliseconds>(steady_clock::now() - last_pred_time).count() >= 50; } );

  /* add a task that prints statistics occasionally */
  StatsPrinterTask stats_printer { event_loop };

  stats_printer.add( playback_interface );

  /* run the event loop forever */
  while ( event_loop->wait_next_event( stats_printer.wait_time_ms() ) != EventLoop::Result::Exit ) {
  }
}

void usage_message( const string_view argv0 )
{
  cerr << "Usage: " << argv0 << " [audio_device] [midi_device]\n";

  cerr << "Available audio devices:";

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
