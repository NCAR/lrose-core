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
// Helpers.cc: helper objects for moments computations
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


#include <cassert>

#include "Helpers.hh"

using namespace std;

// Global variables - instance

Helpers *Helpers::_instance = NULL;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

Helpers::Helpers()
{

}

//////////////////////////////////////////////////////////////////////////
// Destructor

Helpers::~Helpers()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

Helpers &Helpers::inst()
{

  if (_instance == (Helpers *)NULL) {
    _instance = new Helpers;
  }

  return *_instance;
}

//////////////////////////////////////////////////////////////////////////
// fft constructor

Helpers::Fft::Fft(int nSamples) :
        _nSamples(nSamples)
        
{

  fft.init(nSamples);
  fftHalf.init(nSamples/2);

}

//////////////////////////////////////////////////////////////////////////
// Get an Fft

const Helpers::Fft &Helpers::getFft(int nSamples)

{

  // if we already have the required fft, return it

  int key = nSamples;
  fftMap_t::iterator it = _fftMap.find(key);
  if (it != _fftMap.end()) {
    return *it->second;
  }

  // create a new fft, intert it into the map and return it

  Helpers::Fft *fft = new Helpers::Fft(nSamples);
  _fftMap.insert(pair<int, Helpers::Fft*>(key, fft));
  return *fft;
  
}

//////////////////////////////////////////////////////////////////////////
// staggered fft constructor

Helpers::FftStag::FftStag(int nSamples, int stagM, int stagN) :
        _nSamples(nSamples),
        _stagM(stagM),
        _stagN(stagN)
        
{

  _nStaggered =
    RadarMoments::computeNExpandedStagPrt(_nSamples, _stagM, _stagN);

  fft.init(_nStaggered);

}

//////////////////////////////////////////////////////////////////////////
// Get a staggered Fft

const Helpers::FftStag& Helpers::getFftStag(int nSamples, int stagM, int stagN)

{
  
  // if we already have the required fft, return it

  int key = nSamples + stagM * 100000 + stagN * 1000000;
  fftStagMap_t::iterator it = _fftStagMap.find(key);
  if (it != _fftStagMap.end()) {
    return *it->second;
  }

  // create a new stag fft, intert it into the map and return it

  Helpers::FftStag *fft = new Helpers::FftStag(nSamples, stagM, stagN);
  _fftStagMap.insert(pair<int, Helpers::FftStag*>(key, fft));
  return *fft;
  
}

//////////////////////////////////////////////////////////////////////////
// regr constructor

Helpers::Regr::Regr(int nSamples, int order) :
        _nSamples(nSamples),
        _order(order)
        
{

  regr.setup(_nSamples, _order);
  regrHalf.setup(_nSamples/2, _order);

}

//////////////////////////////////////////////////////////////////////////
// Get a regr object

const Helpers::Regr& Helpers::getRegr(int nSamples, int order)

{
  
  // if we already have the required fft, return it

  int key = nSamples + order * 10000000;
  regrMap_t::iterator it = _regrMap.find(key);
  if (it != _regrMap.end()) {
    return *it->second;
  }

  // create a new regr, intert it into the map and return it

  Helpers::Regr *regr = new Helpers::Regr(nSamples, order);
  _regrMap.insert(pair<int, Helpers::Regr*>(key, regr));
  return *regr;
  
}

//////////////////////////////////////////////////////////////////////////
// staggered regr constructor

Helpers::RegrStag::RegrStag(int nSamples, int stagM, int stagN, int order) :
        _nSamples(nSamples),
        _stagM(stagM),
        _stagN(stagN),
        _order(order)
        
{
  regr.setupStaggered(_nSamples, _stagM, _stagN, _order);
}

//////////////////////////////////////////////////////////////////////////
// Get a staggered regr object

const Helpers::RegrStag& Helpers::getRegrStag(int nSamples,
                                              int stagM, int stagN, int order)

{
  
  // if we already have the required fft, return it

  int key = nSamples + stagM * 100000 + stagN * 1000000 + order * 10000000;
  regrStagMap_t::iterator it = _regrStagMap.find(key);
  if (it != _regrStagMap.end()) {
    return *it->second;
  }

  // create a new staggered regr, intert it into the map and return it

  Helpers::RegrStag *regr = new Helpers::RegrStag(nSamples, stagM, stagN, order);
  _regrStagMap.insert(pair<int, Helpers::RegrStag*>(key, regr));
  return *regr;
  
}

