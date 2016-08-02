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

#include <string>
#include <vector>
#include <fftw3.h>
#include <radar/RadarComplex.hh>

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

  void fwd(const RadarComplex_t *in, RadarComplex_t *out) const;

  // perform inverse fft

  void inv(const RadarComplex_t *in, RadarComplex_t *out) const;

  // get references to sin and cos arrays
  // will be loaded as required

  const vector<vector<double> > &getCosArray() const;
  const vector<vector<double> > &getSinArray() const;

protected:
  
private:

  int _n;
  double _sqrtN;
  fftw_plan _fftFwd;
  fftw_plan _fftBck;
  fftw_complex *_in;
  fftw_complex *_out;
  
  mutable vector<vector<double> > _cosArray;
  mutable vector<vector<double> > _sinArray;

  void _free();

};

#endif




