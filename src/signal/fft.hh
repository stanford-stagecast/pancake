#pragma once

#include "signal.hh"

#include <complex>
#include <fftw3.h> // must be included last, after <complex>
#include <string>

class FFTW
{
  bool threaded_;

public:
  FFTW( const bool threaded = false );
  ~FFTW();

  void load_wisdom( const std::string_view str );
  std::string save_wisdom();
};

class FFTPlan
{
  size_t size_;
  fftw_plan plan_;

public:
  FFTPlan( const size_t size, const int sign, const bool make_new_plan );
  ~FFTPlan();
  void execute( const Signal& input, Signal& output ) const;

  FFTPlan( const FFTPlan& other ) = delete;
  FFTPlan& operator=( const FFTPlan& other ) = delete;
};

class ForwardFFT
{
  FFTPlan plan_;

public:
  ForwardFFT( const size_t size )
    : plan_( size, FFTW_FORWARD, false )
  {}

  void execute( const TimeDomainSignal& input, BasebandFrequencyDomainSignal& output ) const;
};

class ReverseFFT
{
  FFTPlan plan_;

public:
  ReverseFFT( const size_t size )
    : plan_( size, FFTW_BACKWARD, false )
  {}

  void execute( const BasebandFrequencyDomainSignal& input, TimeDomainSignal& output ) const;
};

TimeDomainSignal delay( const TimeDomainSignal& signal, const double tau );

TimeDomainSignal delay( const TimeDomainSignal& signal,
                        const double tau,
                        const ForwardFFT& fft,
                        const ReverseFFT& ifft );
