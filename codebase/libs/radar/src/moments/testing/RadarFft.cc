// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// RadarFft.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// RadarFft handles Fast Fourier Transform computations
//
////////////////////////////////////////////////////////////////

#include "RadarFft.hh"
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>
using namespace std;

// Constructors

RadarFft::RadarFft()
  
{

  _n = 0;
  _sqrtN = 0;
  _in = NULL;
  _out = NULL;
  _tmp = NULL;

}

void RadarFft::init(int n)
  
{

  if (_n == n) {
    return;
  } else {
    _free();
  }

  assert(n != 0);

  _sqrtN = sqrt((double) n);
  
  // set up Fft plans

  if (_n > 0) {
    if (_in) fftw_free(_in);
    if (_out) fftw_free(_out);
  }
  
  _in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * n);
  _out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * n);
  _tmp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * n);

  _fftFwd = fftw_plan_dft_1d(n, _in, _out, FFTW_FORWARD, FFTW_MEASURE);
  _fftBck = fftw_plan_dft_1d(n, _in, _out, FFTW_BACKWARD, FFTW_MEASURE);

  _n = n;

}

RadarFft::RadarFft(int n) :
        _n(n)
  
{

  assert(_n != 0);
  
  _sqrtN = sqrt((double) _n);
  
  // set up Fft plans

  _in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _n);
  _out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _n);
  _tmp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _n);

  _fftFwd = fftw_plan_dft_1d(_n, _in, _out, FFTW_FORWARD, FFTW_MEASURE);
  _fftBck = fftw_plan_dft_1d(_n, _in, _out, FFTW_BACKWARD, FFTW_MEASURE);

}

// destructor

RadarFft::~RadarFft()

{

  _free();

}

///////////////////////////////////////////////////
// free up

void RadarFft::_free()
  
{

  if (_n == 0) {
    return;
  }

  fftw_destroy_plan(_fftFwd);
  fftw_destroy_plan(_fftBck);
  
  if (_in) {
    fftw_free(_in);
    _in = NULL;
  }
  
  if (_out) {
    fftw_free(_out);
    _out = NULL;
  }

  if (_tmp) {
    fftw_free(_tmp);
    _tmp = NULL;
  }

}


///////////////////////////////////////////////
// compute forward

void RadarFft::fwd(const complex<double> *in, complex<double> *out) const
  
{

  assert(_n != 0);
  
  memcpy(_in, in, _n * sizeof(complex<double>));
  fftw_execute(_fftFwd);

  // adjust by sqrt(n)

  double *oo = (double *) _out;
  for (int ii = 0; ii < _n; ii++) {
    double real = *oo / _sqrtN;
    oo++;
    double imag = *oo / _sqrtN;
    oo++;
    out[ii] = complex<double>(real, imag);
  }

}

///////////////////////////////////////////////
// compute inverse

void RadarFft::inv(const complex<double> *in, complex<double> *out) const
  
{
  
  assert(_n != 0);
  
  memcpy(_in, in, _n * sizeof(complex<double>));
  fftw_execute(_fftBck);

  // adjust by sqrt(n)

  double *oo = (double *) _out;
  for (int ii = 0; ii < _n; ii++) {
    double real = *oo / _sqrtN;
    oo++;
    double imag = *oo / _sqrtN;
    oo++;
    out[ii] = complex<double>(real, imag);
  }

}

/////////////////////////////////////////////////////////////////
// Shift a spectrum, in place, so that DC is in the center.
// Swaps left and right sides.
// DC location location starts at index 0.
// After the shift:
//   if n is odd,  the DC location is at the center index
//   if n is even, the DC location is at index n/2

void RadarFft::shift(complex<double> *spectrum) const
  
{

  assert(_n != 0);

  int nRight = _n / 2;
  int nLeft = _n - nRight;

  RadarFft::copy(_tmp, spectrum, nLeft);
  RadarFft::copy(spectrum, spectrum + nLeft, nRight);
  RadarFft::copy(spectrum + nRight, _tmp, nLeft);
  
  // memcpy(_tmp, spectrum, nLeft * sizeof(complex<double>));
  // memcpy(spectrum, spectrum + nLeft, nRight * sizeof(complex<double>));
  // memcpy(spectrum + nRight, _tmp, nLeft * sizeof(complex<double>));

}

/////////////////////////////////////////////////////////////////
// Unshift a spectrum, in place, to undo a previous shift.
// Swaps left and right sides.
// After the shift, DC is at index 0.

void RadarFft::unshift(complex<double> *spectrum) const
  
{

  assert(_n != 0);

  int nRight = _n / 2;
  int nLeft = _n - nRight;
  
  RadarFft::copy(_tmp, spectrum, nRight);
  RadarFft::copy(spectrum, spectrum + nRight, nLeft);
  RadarFft::copy(spectrum + nLeft, _tmp, nRight);
  
  // memcpy(_tmp, spectrum, nRight * sizeof(complex<double>));
  // memcpy(spectrum, spectrum + nRight, nLeft * sizeof(complex<double>));
  // memcpy(spectrum + nLeft, _tmp, nRight * sizeof(complex<double>));

}

//////////////////////////////////
// get reference to cosine array
// will be computed if needed

const vector<vector<double> > &RadarFft::getCosArray() const

{

  if ((int) _cosArray.size() == _n) {
    return _cosArray;
  }

  _cosArray.clear();

  for (int ii = 0; ii < _n; ii++) {
    vector<double> cosVec;
    for (int jj = 0; jj < _n; jj++) {
      double angleRad = 2.0 * M_PI * jj * ii / (double) _n;
      cosVec.push_back(cos(angleRad));
    }
    _cosArray.push_back(cosVec);
  }
  
  return _cosArray;

}

//////////////////////////////////
// get reference to sine array
// will be computed if needed

const vector<vector<double> > &RadarFft::getSinArray() const

{

  if ((int) _sinArray.size() == _n) {
    return _sinArray;
  }

  _sinArray.clear();

  for (int ii = 0; ii < _n; ii++) {
    vector<double> sinVec;
    for (int jj = 0; jj < _n; jj++) {
      double angleRad = 2.0 * M_PI * jj * ii / (double) _n;
      sinVec.push_back(sin(angleRad));
    }
    _sinArray.push_back(sinVec);
  }
  
  return _sinArray;

}

/////////////////////////////////////////////
// copy between complex<double> and fftw_complex

void RadarFft::copy(fftw_complex *dest,
                    const complex<double> *src,
                    size_t nn)
  
{
  for (size_t ii = 0; ii < nn; ii++) {
    dest[ii][0] = src[ii].real();
    dest[ii][1] = src[ii].imag();
  }
}

void RadarFft::copy(complex<double> *dest,
                    const fftw_complex *src,
                    size_t nn)
  
{
  for (size_t ii = 0; ii < nn; ii++) {
    dest[ii] = complex<double>(src[ii][0], src[ii][1]);
  }
}

void RadarFft::copy(complex<double> *dest,
                    const complex<double> *src,
                    size_t nn)
  
{
  for (size_t ii = 0; ii < nn; ii++) {
    dest[ii] = src[ii];
  }
}

void RadarFft::copy(fftw_complex *dest,
                    const fftw_complex *src,
                    size_t nn)
  
{
  for (size_t ii = 0; ii < nn; ii++) {
    dest[ii][0] = src[ii][0];
    dest[ii][1] = src[ii][1];
  }
}

//////////////////////////////////////////////////////
// compute DFT

void RadarFft::computeDft(const vector<double> &inReal,
                          const vector<double> &inImag,
                          vector<double> &outReal,
                          vector<double> &outImag)
  
{
	
  size_t nn = inReal.size();

  for (size_t kk = 0; kk < nn; kk++) {  // For each output element
    double sumReal = 0.0;
    double sumImag = 0.0;
    for (size_t tt = 0; tt < nn; tt++) {  // For each input element
      double angle = 2.0 * M_PI * tt * kk / nn;
      sumReal +=  inReal[tt] * cos(angle) + inImag[tt] * sin(angle);
      sumImag += -inReal[tt] * sin(angle) + inImag[tt] * cos(angle);
    }
    outReal[kk] = sumReal;
    outImag[kk] = sumImag;
  }

}

void RadarFft::computeDft(const vector<complex<double>> &in,
                          vector<complex<double>> &out)
  
{
	
  size_t nn = in.size();

  for (size_t kk = 0; kk < nn; kk++) {  // For each output element
    double sumReal = 0.0;
    double sumImag = 0.0;
    for (size_t tt = 0; tt < nn; tt++) {  // For each input element
      double angle = 2.0 * M_PI * tt * kk / nn;
      sumReal +=  in[tt].real() * cos(angle) + in[tt].imag() * sin(angle);
      sumImag += -in[tt].real() * sin(angle) + in[tt].imag() * cos(angle);
    }
    out[kk] = complex<double>(sumReal, sumImag);
  }
  
}

