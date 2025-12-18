// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// ScalarsThread.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// Thread class for handling computations
//
///////////////////////////////////////////////////////////////

#include <cassert>
#include <radar/KdpFiltParams.hh>
#include <radar/NcarPidParams.hh>
#include "RadxCartDP.hh"
#include "ScalarsThread.hh"
#include "ScalarsCompute.hh"
#include "Params.hh"

///////////////////////////////////////////////////////////////
// Constructor

ScalarsThread::ScalarsThread(RadxCartDP *parent,
                             const Params &params,
                             const KdpFiltParams &kdpFiltParams,
                             const NcarPidParams &ncarPidParams,
                             const PrecipRateParams &precipRateParams,
                             int threadNum) :
        _parent(parent),
        _params(params),
        _kdpFiltParams(kdpFiltParams),
        _ncarPidParams(ncarPidParams),
        _precipRateParams(precipRateParams),
        _threadNum(threadNum)
{

  OK = TRUE;
  _inputRay = NULL;
  _outputRay = NULL;
  
  // create compute compute object
  
  _scalarsCompute = new ScalarsCompute(_parent,
                                       _params,
                                       _kdpFiltParams,
                                       _ncarPidParams,
                                       _precipRateParams,
                                       _threadNum);
  
  if (_scalarsCompute == NULL) {
    OK = FALSE;
    return;
  }
  if (!_scalarsCompute->OK) {
    OK = FALSE;
    _scalarsCompute = NULL;
    return;
  }
  
}  

// Destructor

ScalarsThread::~ScalarsThread()
{
  
  if (_scalarsCompute != NULL) {
    delete _scalarsCompute;
  }
  
}  

// run method

void ScalarsThread::run()
{
  
  // check

  assert(_scalarsCompute != NULL);
  assert(_inputRay != NULL);
  
  // Compute compute object will create the output ray
  // The ownership of the ray is passed to the parent object
  // which adds it to the output volume.

  _outputRay = _scalarsCompute->doCompute(_inputRay,
                                          _parent->getRadarHtKm(),
                                          _parent->getWavelengthM());
  
}
