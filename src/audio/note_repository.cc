#include "note_repository.hh"
#include "wav_wrapper.hh"

#include <cmath>
#include <iostream>

using namespace std;

constexpr float LOW_XFOUT_LOVEL = 8;   // Equivalent to MED_XFIN_LOVEL
constexpr float LOW_XFOUT_HIVEL = 59;  // Equivalent to MED_XFIN_HIVEL
constexpr float HIGH_XFIN_LOVEL = 67;  // Equivalent to MED_XFOUT_LOVEL
constexpr float HIGH_XFIN_HIVEL = 119; // Equivalent to MED_XFOUT_HIVEL

NoteRepository::NoteRepository( const string& sample_directory )
{
  add_notes( sample_directory, "A0", 2 );
  add_notes( sample_directory, "C1", 3 );
  add_notes( sample_directory, "D#1", 3 );
  add_notes( sample_directory, "F#1", 3 );
  add_notes( sample_directory, "A1", 3 );
  add_notes( sample_directory, "C2", 3 );
  add_notes( sample_directory, "D#2", 3 );
  add_notes( sample_directory, "F#2", 3 );
  add_notes( sample_directory, "A2", 3 );
  add_notes( sample_directory, "C3", 3 );
  add_notes( sample_directory, "D#3", 3 );
  add_notes( sample_directory, "F#3", 3 );
  add_notes( sample_directory, "A3", 3 );
  add_notes( sample_directory, "C4", 3 );
  add_notes( sample_directory, "D#4", 3 );
  add_notes( sample_directory, "F#4", 3 );
  add_notes( sample_directory, "A4", 3 );
  add_notes( sample_directory, "C5", 3 );
  add_notes( sample_directory, "D#5", 3 );
  add_notes( sample_directory, "F#5", 3 );
  add_notes( sample_directory, "A5", 3 );
  add_notes( sample_directory, "C6", 3 );
  add_notes( sample_directory, "D#6", 3 );

  // keys below here do not have dampers
  add_notes( sample_directory, "F#6", 3, false );
  add_notes( sample_directory, "A6", 3, false );
  add_notes( sample_directory, "C7", 3, false );
  add_notes( sample_directory, "D#7", 3, false );
  add_notes( sample_directory, "F#7", 3, false );
  add_notes( sample_directory, "A7", 3, false );
  add_notes( sample_directory, "C8", 2, false );

  cerr << "Added " << notes.size() << " notes\n";
}

const wav_frame_t NoteRepository::get_sample( const bool direction,
                                              const size_t note,
                                              const uint8_t velocity,
                                              const unsigned long offset ) const
{
  if ( direction ) {
    if (!notes.at( note ).getSlow().at_end( offset )) {
      return notes.at( note ).get_sample( velocity, offset );
    } else {
      return {0, 0};
    }
    
  }

  return notes.at( note ).getRel().view( offset );
}

bool NoteRepository::note_finished( const bool direction,
                                    const size_t note,
                                    const uint8_t velocity,
                                    const unsigned long offset ) const
{
  if ( direction ) {
    if ( velocity <= LOW_XFOUT_LOVEL ) {
      return notes.at( note ).getSlow().at_end( offset );
    } else if ( velocity <= LOW_XFOUT_HIVEL ) {
      return notes.at( note ).getMed().at_end( offset );
    } else if ( velocity <= HIGH_XFIN_LOVEL ) {
      return notes.at( note ).getMed().at_end( offset );
    } else if ( velocity <= HIGH_XFIN_HIVEL ) {
      return notes.at( note ).getFast().at_end( offset );
    } else {
      return notes.at( note ).getFast().at_end( offset );
    }
  }

  return notes.at( note ).getRel().at_end( offset );
}

void NoteRepository::add_notes( const string& sample_directory,
                                const string& name,
                                const unsigned int num_notes,
                                const bool has_damper )
{
  unsigned int note_num_base = notes.size() + 1;
  for ( unsigned int i = 0; i < num_notes; i++ ) {
    unsigned int release_sample_num = note_num_base + i;

    const unsigned int pitch_bend_modulus = ( release_sample_num - 1 ) % 3;

    notes.emplace_back( sample_directory, name, pitch_bend_modulus, release_sample_num, has_damper );
    
  }
}
