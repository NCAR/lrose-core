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
// PhidpProc.cc
//
// Phidp processing
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2016
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <radar/PhidpProc.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <rapmath/stats.h>
using namespace std;

////////////////////////////////////////////////////
// Constructor

PhidpProc::PhidpProc()

{

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;
  
}

//////////////////////////////////////////////////////////////////
// destructor

PhidpProc::~PhidpProc()

{

  _phidp_.free();
  _phidpSdev_.free();
  
}

/////////////////////////////////////////////////
// compute standard deviation of phidp in range
// computed around the circle
//   nGatesData: number of gates in data
//   nGatesSdev: number of gates over which we compute the sdev
//   missingVal: missingDataValue

void PhidpProc::computePhidpSdev(int nGatesData, 
                                 int nGatesSdev,
                                 double *phidp,
                                 double missingVal)
  
{

  // init

  _nGatesData = nGatesData;
  _nGatesSdev = nGatesSdev;
  _missingVal = missingVal;
  _phidp = _phidp_.alloc(_nGatesData);
  _phidpSdev = _phidpSdev_.alloc(_nGatesData);
  
  memcpy(_phidp, phidp, _nGatesData * sizeof(double));
  memset(_phidpSdev, 0, _nGatesData * sizeof(double));

  // compute number of gates in kernel
  
  int nGatesKernel = _nGatesSdev;
  int nGatesHalf = nGatesKernel / 2;
  
  // save phidp to states array
  
  _phidpStates.resize(_nGatesData);
  
  for (int igate = 0; igate < _nGatesData; igate++) {
    PhidpState &state = _phidpStates[igate];
    state.init(_missingVal);
    state.phidp = _phidp[igate];
    if (state.phidp != _missingVal) {
      state.missing = false;
    }
  }

  // compute folding range

  _computePhidpFoldingRange();
  
  // fill in missing data with noise

  for (int igate = 0; igate < _nGatesData; igate++) {
    PhidpState &state = _phidpStates[igate];
    if (state.missing) {
      double randVal = STATS_uniform_gen();
      double noiseVal = randVal * _phidpFoldRange - _phidpFoldVal;
      state.phidp = noiseVal;
      _phidp[igate] = noiseVal;
      state.missing = false;
    }
  }

  // init (x,y) representation of phidp
  
  for (int igate = 0; igate < _nGatesData; igate++) {
    PhidpState &state = _phidpStates[igate];
    if (!state.missing) {
      double phase = state.phidp;
      if (_phidpFoldsAt90) {
        phase *= 2.0;
      }
      double sinVal, cosVal;
      ta_sincos(phase * DEG_TO_RAD, &sinVal, &cosVal);
      state.xx = cosVal;
      state.yy = sinVal;
    }
  }

  // compute mean phidp at each gate

  for (int igate = 0; igate < _nGatesData; igate++) {
  
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sumxx = 0.0;
    double sumyy = 0.0;
    
    for (int jj = igate - nGatesHalf; jj <= igate + nGatesHalf; jj++) {
      if (jj < 0 || jj >= _nGatesData) {
        continue;
      }
      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        cerr << "-";
        continue;
      }
      double xx = jstate.xx;
      double yy = jstate.yy;
      sumxx += xx;
      sumyy += yy;
      count++;
    }
  
    // if (count <= nGatesHalf) {
    //   return;
    // }
    
    istate.meanxx = sumxx / count;
    istate.meanyy = sumyy / count;
    
    double phase = atan2(istate.meanyy, istate.meanxx) * RAD_TO_DEG;
    if (_phidpFoldsAt90) {
      phase *= 0.5;
    }
    istate.phidpMean = phase;

  } // igate
  
  // compute standard deviation at each gate
  // we center on the mean value
  
  for (int igate = 0; igate < _nGatesData; igate++) {
    
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sum = 0.0;
    double sumSq = 0.0;
    
    for (int jj = igate - nGatesHalf; jj <= igate + nGatesHalf; jj++) {
      
      if (jj < 0 || jj >= _nGatesData) {
        continue;
      }

      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        continue;
      }

      // compute difference between this value and the mean

      double diff = jstate.phidp - istate.phidpMean;

      // constrain diff
      
      while (diff < -_phidpFoldVal) {
        diff += 2 * _phidpFoldVal;
      }
      while (diff > _phidpFoldVal) {
        diff -= 2 * _phidpFoldVal;
      }

      // sum up

      count++;
      sum += diff;
      sumSq += diff * diff;
      
    }

    if (count <= nGatesHalf || count < 3) {
      istate.phidpSdev = _missingVal;
    } else {
      double meanDiff = sum / count;
      double term1 = sumSq / count;
      double term2 = meanDiff * meanDiff;
      if (term1 >= term2) {
        istate.phidpSdev = sqrt(term1 - term2);
      }
    }

    _phidpSdev[igate] = istate.phidpSdev;
    
  } // igate
  
}

/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void PhidpProc::_computePhidpFoldingRange()
  
{
  
  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;

  for (int igate = 0; igate < _nGatesData; igate++) {
    if (!_phidpStates[igate].missing) {
      double phidp = _phidpStates[igate].phidp;
      if (phidp < phidpMin) phidpMin = phidp;
      if (phidp > phidpMax) phidpMax = phidp;
    }
  } // igate
  
  _phidpFoldsAt90 = false;
  _phidpFoldVal = 180.0;
  if (phidpMin > -90 && phidpMax < 90) {
    _phidpFoldVal = 90.0;
    _phidpFoldsAt90 = true;
  }
  _phidpFoldRange = _phidpFoldVal * 2.0;
  
  // if values range from (0 -> 360), normalize to (-180 -> 180)
  
  if (phidpMin >= 0 && phidpMax > 180) {
    for (int igate = 0; igate < _nGatesData; igate++) {
      if (!_phidpStates[igate].missing) {
        _phidpStates[igate].phidp -= 180.0;
      }
    }
  }

}

