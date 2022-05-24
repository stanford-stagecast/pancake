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

  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    if (note_repo.note_finished( true, event_note - KEY_OFFSET, event_velocity, i)) break;
    float amp_multi = 0.2; /* to avoid clipping */

    std::pair<float, float> press_samp = note_repo.get_sample( true, event_note - KEY_OFFSET, event_velocity, i );
    std::pair<float, float> curr_samp = k.future.safe_get(i + now);
    press_samp.first *= amp_multi;
    press_samp.second *= amp_multi;
    press_samp.first += curr_samp.first;
    press_samp.second += curr_samp.second;

    k.future.safe_set( i + now, press_samp );
  }
}

void Synthesizer::sim_key_release( uint8_t event_note, const size_t now )
{
  auto& k = keys.at( event_note - KEY_OFFSET );
  float vol_ratio = 1.0;

  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    std::pair<float, float> curr_samp = k.future.safe_get(i + now);
    curr_samp.first *= vol_ratio;
    curr_samp.second *= vol_ratio;

    k.future.safe_set( i + now, curr_samp );

    vol_ratio--;
  }
}

void Synthesizer::calculate_total_future( ChannelPair& future, const size_t now )
{
  //auto& total_future_region = future.region( now, 305 * 4096 );
  //fill( future, 0 );
  

  for ( size_t i = 0; i < 305 * 4096; i++ ) {
    std::pair<float, float> curr_samp = { 0, 0 };

    for ( size_t j = 0; j < NUM_KEYS; j++ ) {
      auto& k = keys.at( j );
      std::pair<float, float> key_samp = k.future.safe_get( i );
      curr_samp.first += key_samp.first * 0.5;
      curr_samp.second += key_samp.second * 0.5;
      
    }

    future.safe_set( i + now, curr_samp );
  }
}


// void Synthesizer::advance_sample()
// {
//   frames_processed++;
//   for ( size_t i = 0; i < NUM_KEYS; i++ ) {
//     auto& k = keys.at( i );
//     size_t active_presses = k.presses.size();
//     size_t active_releases = k.releases.size();

//     for ( size_t j = 0; j < active_presses; j++ ) {
//       k.presses.at( j ).offset++;

//       if ( note_repo.note_finished( true, i, k.presses.at( j ).velocity, k.presses.at( j ).offset ) ) {
//         k.presses.erase( k.presses.begin() );
//         j--;
//         active_presses--;
//       } else if ( ( k.presses.at( j ).released && !sustain_down ) & ( k.presses.at( j ).vol_ratio > 0 ) ) {
//         k.presses.at( j ).vol_ratio -= 0.0001;
//       }
//     }

//     for ( size_t j = 0; j < active_releases; j++ ) {
//       k.releases.at( j ).offset++;

//       if ( note_repo.note_finished( false, i, k.releases.at( j ).velocity, k.releases.at( j ).offset ) ) {
//         k.releases.erase( k.releases.begin() );
//         j--;
//         active_releases--;
//       }
//     }
//   }
// }
