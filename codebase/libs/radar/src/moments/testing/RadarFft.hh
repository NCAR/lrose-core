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
/////////////////////////////////////////////////////////////
// RadarFft.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#ifndef RadarFft_hh
#define RadarFft_hh

#include <vector>
#include <fftw3.h>
#include <complex>
using namespace std;

////////////////////////
// This class

class RadarFft {
  
public:
  
  // default constructor - does not initialize
  // You must call init() before using
  
  RadarFft();
  void init(int n);
  
  // constructor - initializes for given size
  // note that the constructor and destructor are not
  // thread-safe, but the other methods are thread-safe
  
  RadarFft(int n);
  
  // destructor
  
  ~RadarFft();

  // perform fwd fft

  void fwd(const complex<double> *in, complex<double> *out) const;

  // perform inverse fft

  void inv(const complex<double> *in, complex<double> *out) const;

  // Shift a spectrum, in place, so that DC is in the center.
  // Swaps left and right sides.
  // DC location location starts at index 0.
  // After the shift:
  //   if n is odd,  the DC location is at the center index
  //   if n is even, the DC location is at index n/2
  
  void shift(complex<double> *spectrum) const;
  
  // Unshift a spectrum, in place, to undo a previous shift.
  // Swaps left and right sides.
  // After the shift, DC is at index 0.
  
  void unshift(complex<double> *spectrum) const;
  
  // copy between complex<double> and fftw_complex
  
  static void copy(fftw_complex *dest,
                   const complex<double> *src,
                   size_t nn);
  
  static void copy(complex<double> *dest,
                   const fftw_complex *src,
                   size_t nn);
  
  static void copy(complex<double> *dest,
                   const complex<double> *src,
                   size_t nn);

  static void copy(fftw_complex *dest,
                   const fftw_complex *src,
                   size_t nn);
  

  // get references to sin and cos arrays
  // will be loaded as required
  
  const vector<vector<double> > &getCosArray() const;
  const vector<vector<double> > &getSinArray() const;

  // compute DFT
  
  static void computeDft(const vector<double> &inReal,
                         const vector<double> &inImag,
                         vector<double> &outReal,
                         vector<double> &outImag);
  
  static void computeDft(const vector<complex<double>> &in,
                         vector<complex<double>> &out);
	
protected:
  
private:

  int _n;
  double _sqrtN;
  fftw_plan _fftFwd;
  fftw_plan _fftBck;
  fftw_complex *_in;
  fftw_complex *_out;
  fftw_complex *_tmp;
  
  mutable vector<vector<double> > _cosArray;
  mutable vector<vector<double> > _sinArray;

  void _free();

};

#endif




