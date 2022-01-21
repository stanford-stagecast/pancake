#pragma once

#include <alsa/asoundlib.h>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "audio_buffer.hh"
#include "file_descriptor.hh"

class ALSADevices
{
public:
  struct Device
  {
    std::string name;
    std::vector<std::pair<std::string, std::string>> interfaces;
  };

  static std::vector<Device> list();

  static std::pair<std::string, std::string> find_device( const std::vector<std::string_view> descriptions );
};

class PCMFD : public FileDescriptor
{
public:
  using FileDescriptor::FileDescriptor;

  using FileDescriptor::register_read;
};

struct AudioStatistics
{
  size_t last_recovery;
  unsigned int recoveries;

  unsigned int total_wakeups;
  unsigned int min_delay { std::numeric_limits<unsigned int>::max() };
};

class AudioInterface
{
  std::string interface_name_, annotation_;
  snd_pcm_t* pcm_;

  void check_state( const snd_pcm_state_t expected_state );

  snd_pcm_sframes_t avail_ {}, delay_ {};

  size_t cursor_ {};

  AudioStatistics statistics_ {};

  class Buffer
  {
    snd_pcm_t* pcm_;
    const snd_pcm_channel_area_t* areas_;
    unsigned int frame_count_;
    snd_pcm_uframes_t offset_;

  public:
    Buffer( AudioInterface& interface, const unsigned int sample_count );
    void commit( const unsigned int num_frames );
    void commit() { commit( frame_count_ ); }
    ~Buffer();

    int32_t& sample( const bool right_channel, const unsigned int sample_num )
    {
      return *( static_cast<int32_t*>( areas_[0].addr ) + right_channel + 2 * ( offset_ + sample_num ) );
    }

    /* can't copy or assign */
    Buffer( const Buffer& other ) = delete;
    Buffer& operator=( const Buffer& other ) = delete;

    unsigned int frame_count() const { return frame_count_; }
  };

public:
  struct Configuration
  {
    unsigned int sample_rate { 48000 }; // samples per second
    unsigned int avail_minimum { 24 };  // minimum samples that have to be available in buffer to trigger event
    unsigned int period_size { 48 };    // samples per "period"
    unsigned int buffer_size { 240 };   // size of buffer

    unsigned int start_threshold { 24 }; // how many samples to accumulate before starting playback
  };

private:
  Configuration config_ {};

public:
  AudioInterface( const std::string_view interface_name,
                  const std::string_view annotation,
                  const snd_pcm_stream_t stream );

  void initialize();
  void start();
  void prepare();
  void drop();
  void recover();
  bool update();

  const Configuration& config() const { return config_; }
  void set_config( const Configuration& other ) { config_ = other; }

  const AudioStatistics& statistics() const { return statistics_; }

  snd_pcm_state_t state() const;
  unsigned int avail() const { return avail_; }
  unsigned int delay() const { return delay_; }

  std::string name() const;
  PCMFD fd();

  size_t cursor() const { return cursor_; }

  void play( const size_t play_until_sample, const ChannelPair& playback );

  ~AudioInterface();

  /* can't copy or assign */
  AudioInterface( const AudioInterface& other ) = delete;
  AudioInterface& operator=( const AudioInterface& other ) = delete;
};

inline float float_to_dbfs( const float sample_f )
{
  if ( sample_f <= 0.00001 ) {
    return -100;
  }

  return 20 * log10( sample_f );
}

inline float dbfs_to_float( const float dbfs )
{
  if ( dbfs <= -99 ) {
    return 0.0;
  }

  return exp10( dbfs / 20 );
}
