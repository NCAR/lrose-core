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
// Fft.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////
//
// Fft handles Fast Fourier Transform computations
//
////////////////////////////////////////////////////////////////

#include "Fft.hh"
#include <toolsa/toolsa_macros.h>
#include <iostream>
using namespace std;

// Constructor

Fft::Fft(int n) :
  _n(n)
  
{

  _sqrtN = sqrt((double) _n);
  
  // set up Fft plans

  _in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _n);
  _out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _n);

  _fftFwd = fftw_plan_dft_1d(_n, _in, _out, FFTW_FORWARD, FFTW_MEASURE);
  _fftBck = fftw_plan_dft_1d(_n, _in, _out, FFTW_BACKWARD, FFTW_MEASURE);

}

// destructor

Fft::~Fft()

{

  fftw_destroy_plan(_fftFwd);
  fftw_destroy_plan(_fftBck);
  
  fftw_free(_in);
  fftw_free(_out);

}


///////////////////////////////////////////////
// compute forward

void Fft::fwd(const Complex_t *in, Complex_t *out)
  
{

  memcpy(_in, in, _n * sizeof(Complex_t));
  fftw_execute(_fftFwd);

  // adjust by sqrt(n)

  double *oo = (double *) _out;
  for (int ii = 0; ii < _n; ii++, out++) {
    out->re = *oo / _sqrtN;
    oo++;
    out->im = *oo / _sqrtN;
    oo++;
  }

}

///////////////////////////////////////////////
// compute inverse

void Fft::inv(const Complex_t *in, Complex_t *out)
  
{
  
  memcpy(_in, in, _n * sizeof(Complex_t));
  fftw_execute(_fftBck);

  // adjust by sqrt(n)

  double *oo = (double *) _out;
  for (int ii = 0; ii < _n; ii++, out++) {
    out->re = *oo / _sqrtN;
    oo++;
    out->im = *oo / _sqrtN;
    oo++;
  }

}

