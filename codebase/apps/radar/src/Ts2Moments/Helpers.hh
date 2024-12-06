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
//////////////////////////////////////////////////////////////////////////
// Helpers.hh: helper objects for moments computations
//
// EOL, NCAR, Boulder CO
//
// April 2010
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
//
//////////////////////////////////////////////////////////////////////////

#ifndef Helpers_HH
#define Helpers_HH

#include <map>
#include <radar/RadarFft.hh>
#include <radar/RadarMoments.hh>
#include <radar/ForsytheRegrFilter.hh>
using namespace std;

class Helpers

{

public:

  ~Helpers();
  
  ///////////////////////////////////////////////////////////
  // Inst() - Retrieve the singleton instance of this class.
  
  static Helpers &inst();
  
  // inner classes

  // normal FFT

  class Fft {
  public:
    RadarFft fft;
    RadarFft fftHalf;
    Fft(int nSamples);
  private:
    int _nSamples;
  };

  // staggered FFT
  
  class FftStag {
  public:
    RadarFft fft;
    FftStag(int nSamples, int stagM, int stagN);
  private:
    int _nSamples;
    int _stagM;
    int _stagN;
    int _nStaggered;
  };

  // regression clutter filtering
  
  class Regr {
  public:
    ForsytheRegrFilter regr;
    ForsytheRegrFilter regrHalf;
    Regr(int nSamples, int order);
  private:
    int _nSamples;
    int _order;
  };
  
  // staggered regression clutter filtering
  
  class RegrStag {
  public:
    ForsytheRegrFilter regr;
    RegrStag(int nSamples, int stagM, int stagN, int order);
  private:
    int _nSamples;
    int _stagM;
    int _stagN;
    int _order;
  };

  // get an Fft object

  const Fft &getFft(int nSamples);
  
  // get an FftStag object

  const FftStag &getFftStag(int nSamples, int stagM, int stagN);
  
  // Get a regr object
  
  const Regr& getRegr(int nSamples, int order);

  // Get a staggered regr object
  
  const RegrStag& getRegrStag(int nSamples, int stagM, int stagN, int order);

private:

  // Singleton instance pointer

  static Helpers *_instance;
  
  //////////////////////////////////////////////////////////////////
  // Constructor -- private because this is a singleton object

  Helpers();

  // maps

  typedef map<int, Fft*> fftMap_t;
  fftMap_t _fftMap;

  typedef map<int, FftStag*> fftStagMap_t;
  fftStagMap_t _fftStagMap;

  typedef map<int, Regr*> regrMap_t;
  regrMap_t _regrMap;

  typedef map<int, RegrStag*> regrStagMap_t;
  regrStagMap_t _regrStagMap;

};


#endif
