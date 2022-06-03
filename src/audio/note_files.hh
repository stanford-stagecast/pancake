#pragma once

#include "wav_wrapper.hh"

#include <vector>

class NoteFiles
{
  WavWrapper slow;
  WavWrapper med;
  WavWrapper fast;
  WavWrapper rel;

  bool has_damper_;

  std::vector<std::vector<std::pair<float, float>>> velocity_samps {};

public:
  NoteFiles( const std::string& sample_directory,
             const std::string& note,
             const unsigned int pitch_bend_modulus,
             const size_t key_num,
             const bool has_damper );

  const WavWrapper& getSlow() const { return slow; };
  const WavWrapper& getMed() const { return med; };
  const WavWrapper& getFast() const { return fast; };
  const WavWrapper& getRel() const { return rel; };

  bool has_damper() const { return has_damper_; }
  std::pair<float, float> get_sample( uint8_t velocity, size_t offset ) const { 
    if (velocity > 27) velocity = 27;
    return velocity_samps.at( (size_t) velocity ).at( offset ); 
  };

  void bend_pitch( const double pitch_bend_ratio );
  void calculate_velocity_samps( size_t velocity );
};
