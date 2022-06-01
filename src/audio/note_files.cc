#include "note_files.hh"
#include <iostream>
#include <cmath>

using namespace std;

static const string suff_slow = "v1-PA.wav";
static const string suff_med = "v8.5-PA.wav";
static const string suff_fast = "v16.wav";

constexpr float LOW_XFOUT_LOVEL = 8;   // Equivalent to MED_XFIN_LOVEL
constexpr float LOW_XFOUT_HIVEL = 59;  // Equivalent to MED_XFIN_HIVEL
constexpr float HIGH_XFIN_LOVEL = 67;  // Equivalent to MED_XFOUT_LOVEL
constexpr float HIGH_XFIN_HIVEL = 119; // Equivalent to MED_XFOUT_HIVEL

NoteFiles::NoteFiles( const string& sample_directory,
                      const string& note,
                      const unsigned int pitch_bend_modulus,
                      const size_t key_num,
                      const bool has_damper )
  : slow( sample_directory + note + suff_slow )
  , med( sample_directory + note + suff_med )
  , fast( sample_directory + note + suff_fast )
  , rel( sample_directory + "rel" + to_string( key_num ) + ".wav" )
  , has_damper_( has_damper )
{
  /* do we need to bend the pitch? */

    if ( pitch_bend_modulus == 0 ) {
      std::cerr << "NOT bending for: " << note << " = " << key_num << "\n";
    } else if ( pitch_bend_modulus == 1 ) {
      std::cerr << "Bending UP from " << note << "\n";
      bend_pitch( pow( 2, -1.0 / 12.0 ) );
    } else {
      std::cerr << "Bending DOWN from " << note << "\n";
      bend_pitch( pow( 2, 1.0 / 12.0 ) );
    }

    for (size_t i = 0; i < 28; i++) {
      calculate_velocity_samps( i );
    }
}

void NoteFiles::bend_pitch( const double pitch_bend_ratio )
{
  slow.bend_pitch( pitch_bend_ratio );
  med.bend_pitch( pitch_bend_ratio );
  fast.bend_pitch( pitch_bend_ratio );
}

void NoteFiles::calculate_velocity_samps( size_t velocity ) {
  vector<pair<float, float>> new_samples{};
  size_t offset = 0;
  if ( velocity <= LOW_XFOUT_LOVEL ) {
    while (!getSlow().at_end( offset )) {
      new_samples.push_back(getSlow().view( offset ));
      offset++;
    }
      
  } else if ( velocity <= LOW_XFOUT_HIVEL ) {
    while (!getMed().at_end( offset )) {
      std::pair<float, float> new_samp = getSlow().view( offset );
      new_samp.first *= ( LOW_XFOUT_HIVEL - velocity ) / ( LOW_XFOUT_HIVEL - LOW_XFOUT_LOVEL );
      new_samp.second *= ( LOW_XFOUT_HIVEL - velocity ) / ( LOW_XFOUT_HIVEL - LOW_XFOUT_LOVEL );

      std::pair<float, float> med_samp = getMed().view( offset );
      new_samp.first += med_samp.first * ( ( velocity - LOW_XFOUT_LOVEL ) / ( LOW_XFOUT_HIVEL - LOW_XFOUT_LOVEL ) );
      new_samp.second
        += med_samp.second * ( ( velocity - LOW_XFOUT_LOVEL ) / ( LOW_XFOUT_HIVEL - LOW_XFOUT_LOVEL ) );
      new_samples.push_back(new_samp);
      offset++;
    }
    
  } else if ( velocity <= HIGH_XFIN_LOVEL ) {
    while (!getMed().at_end( offset )) {
      new_samples.push_back(getMed().view( offset ));
      offset++;
    }

  } else if ( velocity <= HIGH_XFIN_HIVEL ) {
    while (!getFast().at_end( offset )) {
      std::pair<float, float> new_samp = getMed().view( offset );
      new_samp.first *= ( HIGH_XFIN_HIVEL - velocity ) / ( HIGH_XFIN_HIVEL - HIGH_XFIN_LOVEL );
      new_samp.second *= ( HIGH_XFIN_HIVEL - velocity ) / ( HIGH_XFIN_HIVEL - HIGH_XFIN_LOVEL );

      std::pair<float, float> fast_samp = getFast().view( offset );
      new_samp.first
        += fast_samp.first * ( ( velocity - HIGH_XFIN_LOVEL ) / ( HIGH_XFIN_HIVEL - HIGH_XFIN_LOVEL ) );
      new_samp.second
        += fast_samp.second * ( ( velocity - HIGH_XFIN_LOVEL ) / ( HIGH_XFIN_HIVEL - HIGH_XFIN_LOVEL ) );

      new_samples.push_back(new_samp);
      offset++;
    }
    
  } else {
    while (!getFast().at_end( offset )) {
      new_samples.push_back(getFast().view( offset ));
      offset++;
    }
  }
  velocity_samps.push_back(new_samples);
}
