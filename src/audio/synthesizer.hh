#pragma once

#include "midi_processor.hh"
#include "note_repository.hh"
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

    std::vector<sound> presses;
    std::vector<sound> releases;
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

  wav_frame_t calculate_curr_sample() const;

  void advance_sample();
};
