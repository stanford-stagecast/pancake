#include "midi_processor.hh"

#include <iostream>

using namespace std;
using namespace chrono;

void MidiProcessor::pop_active_sense_bytes()
{
  while ( unprocessed_midi_bytes_.readable_region().size()
          and uint8_t( unprocessed_midi_bytes_.readable_region().at( 0 ) ) == 0xfe ) {
    unprocessed_midi_bytes_.pop( 1 );
  }
}

void MidiProcessor::read_from_fd( FileDescriptor& fd )
{
  unprocessed_midi_bytes_.push_from_fd( fd );

  pop_active_sense_bytes();

  last_event_time_ = steady_clock::now();
}

void MidiProcessor::create_test_input( FileDescriptor& fd )
{
  //const auto now = steady_clock::now();

  if ( !test_pressed ) {
    for ( size_t r = 0; r < 2; r++) {
      for ( size_t i = 25; i < 30; i++ ) {
        fd.write(std::string(1, (char) 144));
        fd.write(std::string(1, (char) i));
        fd.write(std::string(1, (char) 65));
      }
    }
    
    test_pressed = true;
    time_of_test_press = steady_clock::now();
  } else {   
    // for ( size_t r = 0; r < 2; r++) {
    //   for ( size_t i = 25; i < 30; i++ ) {
    //     fd.write(std::string(1, (char) 128));
    //     fd.write(std::string(1, (char) i));
    //     fd.write(std::string(1, (char) 65));
    //   }
    // }
    // test_pressed = false;
  }

  pop_active_sense_bytes();

  
}

void MidiProcessor::pop_event()
{
  while ( unprocessed_midi_bytes_.readable_region().size() >= 3 ) {
    unprocessed_midi_bytes_.pop( 3 );
    pop_active_sense_bytes();
  }
}

bool MidiProcessor::data_timeout() const
{
  const auto now = steady_clock::now();

  return duration_cast<milliseconds>( now - last_event_time_ ).count() > 1000;
}
