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
// RadarFftDouble.cc
//
// Real-valued FFT support using FFTW.
//
/////////////////////////////////////////////////////////////

#include <radar/RadarFftDouble.hh>

#include <cassert>
#include <cmath>
#include <cstring>

pthread_mutex_t RadarFftDouble::_initMutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////////////////////////////////////////
// Default constructor

RadarFftDouble::RadarFftDouble() :
        _n(0),
        _nSpectrum(0),
        _sqrtN(0.0),
        _real(NULL),
        _complex(NULL),
        _fftFwd(NULL),
        _fftBck(NULL)
{
}

/////////////////////////////////////////////////////////////
// Constructor initialized for n samples

RadarFftDouble::RadarFftDouble(int n) :
        _n(0),
        _nSpectrum(0),
        _sqrtN(0.0),
        _real(NULL),
        _complex(NULL),
        _fftFwd(NULL),
        _fftBck(NULL)
{
  init(n);
}

/////////////////////////////////////////////////////////////
// Destructor

RadarFftDouble::~RadarFftDouble()
{
  _free();
}

/////////////////////////////////////////////////////////////
// Initialize or reinitialize

void RadarFftDouble::init(int n)
{
  assert(n > 0);

  if (_n == n) {
    return;
  }

  _free();

  _n = n;
  _nSpectrum = n / 2 + 1;
  _sqrtN = std::sqrt(static_cast<double>(n));

  // FFTW plan creation and destruction are not inherently thread-safe.

  pthread_mutex_lock(&_initMutex);

  _real = static_cast<double *>(fftw_malloc(sizeof(double) * _n));
  _complex = static_cast<fftw_complex *>(
      fftw_malloc(sizeof(fftw_complex) * _nSpectrum));

  assert(_real != NULL);
  assert(_complex != NULL);

  _fftFwd = fftw_plan_dft_r2c_1d(_n, _real, _complex, FFTW_MEASURE);
  _fftBck = fftw_plan_dft_c2r_1d(_n, _complex, _real, FFTW_MEASURE);

  assert(_fftFwd != NULL);
  assert(_fftBck != NULL);

  pthread_mutex_unlock(&_initMutex);
}

/////////////////////////////////////////////////////////////
// Free resources

void RadarFftDouble::_free()
{
  if (_n == 0) {
    return;
  }

  pthread_mutex_lock(&_initMutex);

  if (_fftFwd != NULL) {
    fftw_destroy_plan(_fftFwd);
    _fftFwd = NULL;
  }

  if (_fftBck != NULL) {
    fftw_destroy_plan(_fftBck);
    _fftBck = NULL;
  }

  if (_real != NULL) {
    fftw_free(_real);
    _real = NULL;
  }

  if (_complex != NULL) {
    fftw_free(_complex);
    _complex = NULL;
  }

  _n = 0;
  _nSpectrum = 0;
  _sqrtN = 0.0;

  pthread_mutex_unlock(&_initMutex);
}

/////////////////////////////////////////////////////////////
// Forward real-to-complex FFT

void RadarFftDouble::fwd(const double *in, RadarComplex_t *out) const
{
  assert(_n > 0);
  assert(in != NULL);
  assert(out != NULL);

  std::memcpy(_real, in, sizeof(double) * _n);
  fftw_execute(_fftFwd);

  for (int ii = 0; ii < _nSpectrum; ++ii) {
    out[ii].re = _complex[ii][0] / _sqrtN;
    out[ii].im = _complex[ii][1] / _sqrtN;
  }
}

/////////////////////////////////////////////////////////////
// Inverse complex-to-real FFT

void RadarFftDouble::inv(const RadarComplex_t *in, double *out) const
{
  assert(_n > 0);
  assert(in != NULL);
  assert(out != NULL);

  for (int ii = 0; ii < _nSpectrum; ++ii) {
    _complex[ii][0] = in[ii].re;
    _complex[ii][1] = in[ii].im;
  }

  fftw_execute(_fftBck);

  for (int ii = 0; ii < _n; ++ii) {
    out[ii] = _real[ii] / _sqrtN;
  }
}
