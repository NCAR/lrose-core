// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that
// ** the conditions in the UCAR BSD licence are met.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/////////////////////////////////////////////////////////////
// RadarFftDouble.hh
//
// Real-valued FFT support using FFTW.
//
/////////////////////////////////////////////////////////////

#ifndef RadarFftDouble_hh
#define RadarFftDouble_hh

#include <cstddef>
#include <fftw3.h>
#include <pthread.h>
#include <radar/RadarComplex.hh>

class RadarFftDouble {

public:

  // Default constructor. Call init() before use.

  RadarFftDouble();

  // Constructor initialized for n real-valued input samples.

  explicit RadarFftDouble(int n);

  // Destructor.

  ~RadarFftDouble();

  // Initialize or reinitialize for n real-valued samples.

  void init(int n);

  // Number of real-valued time-domain samples.

  int getN() const { return _n; }

  // Number of non-redundant complex spectral samples returned by fwd().
  // This is n / 2 + 1.

  int getNSpectrum() const { return _nSpectrum; }

  // Forward real-to-complex FFT.
  //
  // in must contain getN() doubles.
  // out must have room for getNSpectrum() RadarComplex_t values.
  //
  // The output is normalized by sqrt(n), matching RadarFft.

  void fwd(const double *in, RadarComplex_t *out) const;

  // Inverse complex-to-real FFT.
  //
  // in must contain getNSpectrum() non-redundant spectral values in the
  // format returned by fwd().
  // out must have room for getN() doubles.
  //
  // The output is normalized by sqrt(n), matching RadarFft. Therefore,
  // inv(fwd(x)) reproduces x, apart from floating-point roundoff.

  void inv(const RadarComplex_t *in, double *out) const;

private:

  // Copying would duplicate FFTW plan and buffer ownership.

  RadarFftDouble(const RadarFftDouble &);
  RadarFftDouble &operator=(const RadarFftDouble &);

  int _n;
  int _nSpectrum;
  double _sqrtN;

  double *_real;
  fftw_complex *_complex;

  fftw_plan _fftFwd;
  fftw_plan _fftBck;

  static pthread_mutex_t _initMutex;

  void _free();

};

#endif
