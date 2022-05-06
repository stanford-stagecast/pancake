#include "fft.hh"
#include "signal.hh"

#include <iostream>
#include <string>

using namespace std;

void make_fft_plans( const char* const size_str )
{
  const unsigned long size = stoul( size_str );

  FFTPlan forward { size, FFTW_FORWARD, true };
  FFTPlan backward { size, FFTW_BACKWARD, true };
}

int main( int argc, char* argv[] )
{
  try {
    ios::sync_with_stdio( false );

    if ( argc <= 0 ) {
      abort();
    }

    if ( argc < 2 ) {
      cerr << "Usage: " << argv[0] << " size...\n";
      return EXIT_FAILURE;
    }

    FFTW fftw_state;

    for ( int i = 1; i < argc; i++ ) {
      make_fft_plans( argv[i] );
    }

    cout << fftw_state.save_wisdom();

    return EXIT_SUCCESS;
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
