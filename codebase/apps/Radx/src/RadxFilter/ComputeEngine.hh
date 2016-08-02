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
// ComputeEngine.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// ComputeEngine computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#ifndef ComputeEngine_HH
#define ComputeEngine_HH

#include "Params.hh"
#include <toolsa/TaArray.hh>
#include <Radx/Radx.hh>
class RadxRay;
class RadxField;
#include <pthread.h>
using namespace std;

class ComputeEngine {
  
public:

  // constructor
  
  ComputeEngine(const Params &params);

  // destructor
  
  ~ComputeEngine();

  // compute the moments for given covariance ray
  // storing results in moments ray
  //
  // Creates moments ray and returns it.
  // It must be freed by caller.
  //
  // Returns NULL on error.
  
  RadxRay *compute(RadxRay *covRay);

  bool OK;
  
protected:
private:

  static const double missingDbl;
  const Params &_params;

  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth;
  double _elevation;

  // field data
  
  int _nGates;
  double _startRangeKm, _gateSpacingKm;
  double _nyquist;
  
  // debug printing

  static pthread_mutex_t _debugPrintMutex;

  // private methods
  
  const RadxField *_getField(const RadxRay *inputRay,
                             const string &fieldName);
  
  RadxField *_applyMedian(const RadxField *inputField,
                          int length);
  
  
  RadxField *_applyTriangular(const RadxField *inputField,
                              int length);
  
  
  void _applyTriangular(int mm,
                        int nGates,
                        const Radx::fl32 *inData,
                        Radx::fl32 *outData,
                        Radx::fl32 missing,
                        int length,
                        const double *wts);
  
  RadxField *_applyLeastSquares(const RadxField *inputField,
                                int length);

  void _applyLeastSquares(int mm,
                          int nGates,
                          const Radx::fl32 *inData,
                          Radx::fl32 *outData,
                          Radx::fl32 missing,
                          int length);
  
  void _setFoldLimits(RadxField *outField);

  double _getFoldAngle(double val,
                       double foldLimitLower,
                       double foldRange);

  double _getFoldValue(double angle,
                       double foldLimitLower,
                       double foldRange);

};

#endif
