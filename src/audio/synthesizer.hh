#pragma once

#include "midi_processor.hh"
#include "note_repository.hh"
#include "audio_buffer.hh"
//#include <deque>
#include <vector>

class Synthesizer
{
  struct sound
  {
    unsigned long offset;
    unsigned long velocity;
    float vol_ratio;
    bool released = false;
  };

  struct key /* rename: string */
  {
    /* ChannelPair giving the next 26-ish seconds of THIS string */
    ChannelPair future { 305 * 4096 };
  };

  NoteRepository note_repo; /* has precomputed array of integer samples for each possible press velocity */
  std::vector<key> keys {};
  bool sustain_down = false;
  size_t frames_processed = 0;

public:
  Synthesizer( const std::string& sample_directory );

  void process_event( ChannelPair& future,
                      const size_t now,
                      uint8_t event_type,
                      uint8_t event_note,
                      uint8_t event_velocity );

  void calculate_total_future( ChannelPair& future, const size_t now );

  void add_key_press( uint8_t event_note, uint8_t event_velocity, const size_t now );
  void sim_key_release( uint8_t event_note, uint8_t event_velocity, const size_t now );
};
