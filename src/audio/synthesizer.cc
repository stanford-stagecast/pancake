#include "synthesizer.hh"
#include <cmath>
#include <iostream>

constexpr unsigned int NUM_KEYS = 88;
constexpr unsigned int KEY_OFFSET = 21;

constexpr unsigned int KEY_DOWN = 144;
constexpr unsigned int KEY_UP = 128;
constexpr unsigned int SUSTAIN = 176;

using namespace std;

Synthesizer::Synthesizer( const string& sample_directory )
  : note_repo( sample_directory )
{
  for ( size_t i = 0; i < NUM_KEYS; i++ ) {
    keys.push_back( {  } );
  }
}

void Synthesizer::process_event( ChannelPair& future,
                                 const size_t now,
                                 uint8_t event_type,
                                 uint8_t event_note,
                                 uint8_t event_velocity )
{
  /* for a press: add the recording of the key press
                  at the appropriate velocity (precomputed)
                  to *that string's* future */
  /* for a release: taper the current future of *that string*
                    to zero after 0.2 seconds. then add the release
                    sound to *that string's* future */

  /* finally: sum up all 88 string futures and write that into the "future"
     given as an argument to this function */

  if ( event_type == SUSTAIN ) {
    // std::cerr << (size_t) midi_processor.get_event_type() << " " << (size_t) event_note << " " <<
    // (size_t)event_velocity << "\n";
    if ( event_velocity == 127 )
      sustain_down = true;
    else
      sustain_down = false;
  } else if ( event_type == KEY_DOWN || event_type == KEY_UP ) {
    bool direction = event_type == KEY_DOWN ? true : false;

    if ( !direction ) {
      sim_key_release( event_note, now );
    } else {
      add_key_press( event_note, event_velocity, now );
    }
  }

  calculate_total_future( future, now );
}


void Synthesizer::add_key_press( uint8_t event_note, uint8_t event_velocity, const size_t now )
{
  auto& k = keys.at( event_note - KEY_OFFSET );
  auto key_region_left = k.future.ch1().region( now, 305 * 4096 );
  auto key_region_right = k.future.ch2().region( now, 305 * 4096 );

  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    if (note_repo.note_finished( true, event_note - KEY_OFFSET, event_velocity, i)) break;
    float amp_multi = 0.2; /* to avoid clipping */

    std::pair<float, float> press_samp = note_repo.get_sample( true, event_note - KEY_OFFSET, event_velocity, i );
    press_samp.first *= amp_multi;
    press_samp.second *= amp_multi;

    key_region_left[i] = press_samp.first;
    key_region_right[i] = press_samp.second;
  }
}

void Synthesizer::sim_key_release( uint8_t event_note, const size_t now )
{
  auto& k = keys.at( event_note - KEY_OFFSET );
  float vol_ratio = 1.0;
  auto key_region_left = k.future.ch1().region( now, 305 * 4096 );
  auto key_region_right = k.future.ch2().region( now, 305 * 4096 );
  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    key_region_left[i] *= vol_ratio;
    key_region_right[i] *= vol_ratio;
    vol_ratio -= 0.001;
  }

}

void Synthesizer::calculate_total_future( ChannelPair& future, const size_t now )
{
  //auto& total_future_region = future.region( now, 305 * 4096 );
  //fill( future, 0 );
  auto total_region_left = future.ch1().region( now, 305 * 4096 );
  auto total_region_right = future.ch1().region( now, 305 * 4096 );
  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    total_region_left[i] = 0;
    total_region_right[i] = 0;
  }

  for ( size_t j = 0; j < NUM_KEYS; j++ ) {
    auto& k = keys.at( j );
    auto key_region_left = k.future.ch1().region( now, 305 * 4096 );
    auto key_region_right = k.future.ch2().region( now, 305 * 4096 );
    for ( size_t i = 0; i < 305 * 4096; i++ ) {
      //if (i > 50 && i < 200 && key_region_left[i] != 0)
      //  std::cerr << "total region left: " << total_region_left[i] << "\n";
      //if (i > 50 && i < 200 && key_region_left[i] != 0)
      //  std::cerr << "key region left: " << key_region_left[i] << "\n";
      total_region_left[i] += key_region_left[i];
      total_region_right[i] += key_region_right[i];
    }

  }
}


void Synthesizer::advance_key_future( const size_t to_pop ) {
  for ( size_t j = 0; j < NUM_KEYS; j++ ) {
    auto& k = keys.at( j );
    k.future.pop_before( to_pop );
  }
}