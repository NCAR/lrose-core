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
// Fft2D.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2014
//
///////////////////////////////////////////////////////////////

#ifndef Fft2D_hh
#define Fft2D_hh

#include <fftw3.h>
#include <dataport/port_types.h>

using namespace std;

////////////////////////
// This class

class Fft2D {
  
public:
  
  // default constructor - does not initialize
  // You must call init() before using
  
  Fft2D();
  void init(int ny, int nx);
  
  // constructor - initializes for given size
  // note that the constructor and destructor are not
  // thread-safe, but the other methods are thread-safe
  
  Fft2D(int ny, int nx);
  
  // destructor
  
  ~Fft2D();

  // perform fwd fft

  void fwd(const fl32 *in, fftw_complex *out);

  // perform inverse fft

  void inv(const fftw_complex *in, fl32 *out);

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
  
  void shift(const fftw_complex *in, fftw_complex *out);
  
protected:
  
private:

  int _ny, _nx, _nxy;
  double _sqrtNxy;
  fftw_plan _fftFwd;
  fftw_plan _fftInv;
  fftw_complex *_in;
  fftw_complex *_out;
  fftw_complex *_tmp;

  void _init();
  void _free();

};

#endif




