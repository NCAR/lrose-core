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
// Fft2D.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Fft2D handles Fast Fourier Transform computations
//
////////////////////////////////////////////////////////////////

#include "Fft2D.hh"
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>
using namespace std;

// Default constructor

Fft2D::Fft2D()
  
{
  _init();
}

// constructor with specified sizes

Fft2D::Fft2D(int ny, int nx)
  
{
  _init();
  init(ny, nx);
}

//////////////
// initialize

void Fft2D::_init()
  
{

  _nx = 0;
  _ny = 0;
  _nxy = 0;
  _sqrtNxy = 0;
  _in = NULL;
  _out = NULL;
  _tmp = NULL;
  
}

void Fft2D::init(int ny, int nx)
  
{

  if (nx == _nx && ny == _ny) {
    return;
  } else {
    _free();
  }

  _nx = nx;
  _ny = ny;
  _nxy = nx * ny;
  assert(_nxy != 0);
  
  _sqrtNxy = sqrt((double) _nxy);
  
  // set up Fft plans
  
  if (_in) fftw_free(_in);
  if (_out) fftw_free(_out);
  if (_tmp) fftw_free(_tmp);
  
  _in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _nxy);
  _out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _nxy);
  _tmp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _nxy);
  
  _fftFwd = fftw_plan_dft_2d(_ny, _nx, _in, _out, FFTW_FORWARD, FFTW_MEASURE);
  _fftInv = fftw_plan_dft_2d(_ny, _nx, _in, _out, FFTW_BACKWARD, FFTW_MEASURE);

}

// destructor

Fft2D::~Fft2D()

{

  _free();

}

///////////////////////////////////////////////////
// free up

void Fft2D::_free()
  
{

  if (_nxy == 0) {
    return;
  }

  fftw_destroy_plan(_fftFwd);
  fftw_destroy_plan(_fftInv);
  
  if (_in) {
    fftw_free(_in);
    _in = NULL;
  }
  
  if (_out) {
    fftw_free(_out);
    _out = NULL;
  }

}


///////////////////////////////////////////////
// compute forward

void Fft2D::fwd(const fl32 *in, fftw_complex *out)
  
{

  // check for validity
  
  assert(_nxy != 0);
  
  // load up input array - just the real, set imaginary to 0

  for (int ii = 0; ii < _nxy; ii++) {
    _in[ii][0] = in[ii];
    _in[ii][1] = 0.01;
  }

  // compute fft

  fftw_execute(_fftFwd);

  // adjust by sqrt(nxy)

  for (int ii = 0; ii < _nxy; ii++) {
    _out[ii][0] /= _sqrtNxy;
    _out[ii][1] /= _sqrtNxy;
  }

  // shift

  // memcpy(out, _out, _nxy * sizeof(fftw_complex));
  shift(_out, out);
  
}

///////////////////////////////////////////////
// compute inverse

void Fft2D::inv(const fftw_complex *in, fl32 *out)
  
{

  // check for validity
  
  assert(_nxy != 0);
  
  // shift input on the way in
  
  shift(in, _in);
  // memcpy(_in, in, _nxy * sizeof(fftw_complex));

  // compute inverse fft
  
  fftw_execute(_fftInv);
  
  // adjust by sqrt(nxy)

  for (int ii = 0; ii < _nxy; ii++) {
    double re = _out[ii][0];
    double im = _out[ii][1];
    out[ii] = sqrt(re * re + im * im) / _sqrtNxy;
  }

}

///////////////////////////////////////////////
// shift array in place
//
// swaps the quadrants as follows:
//
//   -------------       -------------
//   |     |     |       |     |     |
//   |  1  |  2  |       |  4  |  3  |
//   |     |     |       |     |     |
//   ------------- ==>>  -------------
//   |     |     |       |     |     |
//   |  3  |  4  |       |  2  |  1  |
//   |     |     |       |     |     |
//   -------------       -------------
//
// Note - can be done in-place
//  i.e. in and out can be the same array

void Fft2D::shift(const fftw_complex *in, fftw_complex *out)
  
{

  // compute half dimensions, allowing for non-even dimension
  
  int nx1 = _nx / 2; // left half
  int ny1 = _ny / 2; // lower half
  int nx2 = _nx - nx1; // right half

  // copy input array into temporary location

  memcpy(_tmp, in, _nxy * sizeof(fftw_complex));
  
  // swap the quadrants
  
  for (int iy = 0, ii = 0, jj = ny1 * _nx;
       iy < ny1;
       iy++, ii += _nx, jj += _nx) {
    // lower-left to upper-right
    memcpy(out + jj + nx2, _tmp + ii, nx1 * sizeof(fftw_complex));
    // lower-right to upper-left
    memcpy(out + jj, _tmp + ii + nx1, nx2 * sizeof(fftw_complex));
  }

  for (int iy = ny1, ii = ny1 * _nx, jj = 0;
       iy < _ny;
       iy++, ii += _nx, jj += _nx) {
    // upper-left to lower-right
    memcpy(out + jj + nx2, _tmp + ii, nx1 * sizeof(fftw_complex));
    // upper-right to lower-left
    memcpy(out + jj, _tmp + ii + nx1, nx2 * sizeof(fftw_complex));
  }

}
  
