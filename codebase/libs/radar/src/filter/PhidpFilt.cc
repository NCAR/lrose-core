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
// PhidpFilt.cc
//
// Filter utilities for phidp
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
////////////////////////////////////////////////////////////////

#include <toolsa/sincos.h>
#include <toolsa/toolsa_macros.h>
#include <radar/PhidpFilt.hh>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////
// Constructor

PhidpFilt::PhidpFilt()
  
{


  _phidpFoldsAt90 = false;
  _phidpFoldVal = 180.0;
  _phidpFoldRange = _phidpFoldVal * 2.0;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;
  
}

//////////////////////////////////////////////////////////////////
// destructor

PhidpFilt::~PhidpFilt()

{

}

/////////////////////////////////////////////////
// compute texture for PHIDP, over kernel length
// takes account of folding
// nGatesData and phidp passed in
// computes sdev and load into sdevPhidp array
// sdevPhidp array must be allocated by caller
//
// If computeTrendRmse is true, we compute the rmse of
// the residuals from the trend mean instead of sdev
//
// if meanPhidp is not NULL, the mean value is stored there
// if texturePhidp is not NULL, the texture value is stored there

void PhidpFilt::computePhidpSdev(int nGatesData,
                                 int nGatesKernel,
                                 const double *phidp,
                                 double missingValue,
                                 bool computeTrendRmse /* = false */,
                                 double *meanPhidp /* = NULL*/,
                                 double *texturePhidp /* = NULL*/)
  
{

  // compute number of gates in kernel, making sure there is an odd number
  
  int nKernelHalf = nGatesKernel / 2;
  nGatesKernel = nKernelHalf * 2 + 1;

  // save phidp to states array

  _phidpStates.resize(nGatesData);
  
  for (int igate = 0; igate < nGatesData; igate++) {
    PhidpState &state = _phidpStates[igate];
    state.init(missingValue);
    state.phidp = phidp[igate];
    if (state.phidp != missingValue) {
      state.missing = false;
    }
  }

  // compute folding range

  _computeFoldingRange(nGatesData);

  // init (x,y) representation of phidp
  
  for (int igate = 0; igate < nGatesData; igate++) {
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

  for (int igate = 0; igate < nGatesData; igate++) {
  
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sumxx = 0.0;
    double sumyy = 0.0;
    
    for (int jj = igate - nKernelHalf; jj <= igate + nKernelHalf; jj++) {
      if (jj < 0 || jj >= nGatesData) {
        continue;
      }
      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        continue;
      }
      double xx = jstate.xx;
      double yy = jstate.yy;
      sumxx += xx;
      sumyy += yy;
      count++;
    }
  
    if (count <= nKernelHalf) {
      continue;
    }
    
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
  
  for (int igate = 0; igate < nGatesData; igate++) {
    
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sum = 0.0;
    double sumSq = 0.0;
    
    for (int jj = igate - nKernelHalf; jj <= igate + nKernelHalf; jj++) {
      
      if (jj < 0 || jj >= nGatesData) {
        continue;
      }

      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        continue;
      }

      double diff;
      
      if (computeTrendRmse) {
        // compute residual between this value and the mean at that point
        // this yields the RMSE of the residuals from the trend
        diff = jstate.phidp - jstate.phidpMean;
      } else {
        // compute difference between this value and the mean at EACH point
        // this yields the standard deviation
        diff = jstate.phidp - istate.phidpMean;
      }


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
      
    } // jj

    if (count <= nKernelHalf || count < 3) {
      istate.phidpTexture = missingValue;
    } else {
      double meanDiff = sum / count;
      double term1 = sumSq / count;
      double term2 = meanDiff * meanDiff;
      if (term1 >= term2) {
        istate.phidpTexture = sqrt(term1 - term2);
      }
    }

    if (texturePhidp != NULL) {
      texturePhidp[igate] = istate.phidpTexture;
    }
    if (meanPhidp != NULL) {
      meanPhidp[igate] = istate.phidpMean;
    }
    
  } // igate
  
}

/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void PhidpFilt::_computeFoldingRange(int nGatesData)
  
{
  
  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;

  for (int igate = 0; igate < nGatesData; igate++) {
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
    for (int igate = 0; igate < nGatesData; igate++) {
      if (!_phidpStates[igate].missing) {
        _phidpStates[igate].phidp -= 180.0;
      }
    }
  }

}

