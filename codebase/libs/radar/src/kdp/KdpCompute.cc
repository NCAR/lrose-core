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
// KdpCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// Kdp for SBand - based on Bringi code
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <cmath>
#include <cstring>
#include <radar/KdpCompute.hh>
#include <radar/FilterUtils.hh>
#include <toolsa/mem.h>
#include <rapmath/umath.h>
using namespace std;

// Constructor

KdpCompute::KdpCompute()
  
{

  setAltHv();
  _applyMedianFilterToPhidp = false;
  _phidpMedianFilterLen = 5;
  _phidpSlopeFitLen = 20;
  _phidpSdevThreshold = 15.0;
  _zdrSdevThreshold = 2.0;
  _rhohvWxThreshold = 0.7;

  _range = NULL;
  _dbz = NULL;
  _zdr = NULL;
  _phidp = NULL;
  _rhohv = NULL;
  _snr = NULL;
  _sdphidp = NULL;
  _sdzdr = NULL;
  _kdp = NULL;

}

// Destructor

KdpCompute::~KdpCompute()
  
{

}

// set transmit pulsing mode

void KdpCompute::setSlant45() {
  _phidpFoldRange = 360.0;
}
void KdpCompute::setAltHv() {
  _phidpFoldRange = 180.0;
}

/////////////////////////////////////
// compute KDP

int KdpCompute::compute(int nGates,
                       const double *range,
                       const double *dbz,
                       const double *zdr,
                       const double *phidp,
                       const double *rhohv,
                       const double *snr,
                       double missingValue)
  
{

  _missingValue = missingValue;

  // allocate the arrays needed
  // copy input arrays, leaving extra space at the beginning
  // for negative indices and at the end for filtering as required

  _range = _range_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _dbz = _dbz_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _zdr = _zdr_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _phidp = _phidp_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _rhohv = _rhohv_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _snr = _snr_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _sdphidp = _sdphidp_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _sdzdr = _sdzdr_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _kdp = _kdp_.alloc(nGates + EXTRA_GATES) + EXTRA_HALF;
  _valid = _valid_.alloc(nGates);
  
  // copy data to working arrays

  for (int ii = 0; ii < nGates; ii++) {
    if (phidp[ii] != _missingValue) {
      _valid[ii] = true;
    } else {
      _valid[ii] = false;
    }
  }

  memset(_range - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_range, range, nGates * sizeof(double));
  
  memset(_dbz - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_dbz, dbz, nGates * sizeof(double));
  
  memset(_zdr - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_zdr, zdr, nGates * sizeof(double));
  
  memset(_phidp - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_phidp, phidp, nGates * sizeof(double));
  
  memset(_rhohv - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_rhohv, rhohv, nGates * sizeof(double));
  
  memset(_snr - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memcpy(_snr, snr, nGates * sizeof(double));
  
  // initialize computed arrays

  memset(_sdphidp - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memset(_sdzdr - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));
  memset(_kdp - EXTRA_HALF, 0, (nGates + EXTRA_GATES) * sizeof(double));

  // apply median filter as appropriate
  
  if (_applyMedianFilterToPhidp) {
    FilterUtils::applyMedianFilter(_phidp, nGates, _phidpMedianFilterLen);
  }

  // if data is missing, set it to value at previous gate
  // except for phidp

  if (_dbz[0] == _missingValue) {
    _dbz[0] = -32.0;
  }
  if (_zdr[0] == _missingValue) {
    _zdr[0] = 0.0;
  }
  if (_rhohv[0] == _missingValue) {
    _rhohv[0] = 0.0;
  }
  if (_snr[0] == _missingValue) {
    _snr[0] = -10.0;
  }
  
  for (int ii = 1; ii < nGates; ii++) {
    if (_dbz[ii] == _missingValue) {
      _dbz[ii] = _dbz[ii-1];
    }
    if (_zdr[ii] == _missingValue) {
      _zdr[ii] = _zdr[ii-1];
    }
    if (_rhohv[ii] == _missingValue) {
      _rhohv[ii] = _rhohv[ii-1];
    }
    if (_snr[ii] == _missingValue) {
      _snr[ii] = _snr[ii-1];
    }
  }
  
  for (int ii = -EXTRA_HALF; ii < 0; ii++) {
    _dbz[ii] = _dbz[0];
    _zdr[ii] = _zdr[0];
    _rhohv[ii] = _rhohv[0];
    _snr[ii] = _snr[0];
  }
  
  for (int ii = nGates; ii < nGates + EXTRA_HALF; ii++) {
    _dbz[ii] = _dbz[nGates-1];
    _zdr[ii] = _zdr[nGates-1];
    _rhohv[ii] = _rhohv[nGates-1];
    _snr[ii] = _snr[nGates-1];
  }
  
  // allocate and initializa arrays
  
  TaArray<int> goodMask_;
  int *goodMask = goodMask_.alloc(nGates);
  
  for (int ii = 0; ii < nGates; ii++) {
    goodMask[ii] = 0; // 0 is good, 1 is bad
  }
  
  // unfold phidp

  _unfoldPhidp(nGates);

  // loop through gates computing KDP

  for (int ii = 0; ii < nGates; ii++) {
    
    // compute sdev of phidp, using unfiltered data
    
    double mean_phidp, sd_phidp;
    _msr(mean_phidp, sd_phidp, phidp + ii, mgood);
    _sdphidp[ii] = sd_phidp;

    // compute sdev of zdr
    
    double mean_zdr, sd_zdr;
    _msr(mean_zdr, sd_zdr, zdr + ii, mgood);
    _sdzdr[ii] = sd_zdr;

    // check - is this weather?

    if (_rhohv[ii] < _rhohvWxThreshold) {
      continue;
    }
    if (_sdphidp[ii] > _phidpSdevThreshold) {
      continue;
    }
    if (_sdzdr[ii] > _zdrSdevThreshold) {
      continue;
    }
    
    double slope = _computePhidpSlope(ii);
    if (slope != _missingValue) {
      _kdp[ii] = slope / 2.0;
    } else {
      _kdp[ii] = _missingValue;
    }

  } // ii
  
  return 0;

}
  
//////////////////////////////////////////////////
// compute PhiDp slope
//
// returns _missingDbl if not enough data

double KdpCompute::_computePhidpSlope(int gateNum)

{
  
  TaArray<double> xx_;
  double *xx = xx_.alloc(_phidpSlopeFitLen);
  TaArray<double> yy_;
  double *yy = yy_.alloc(_phidpSlopeFitLen);

  int gateCount = 0;
  int validCount = 0;
  int countFirstQuarter = 0;
  int countLastQuarter = 0;
  int nGatesHalf =_phidpSlopeFitLen / 2 - 1;
  int nGatesQuarter = nGatesHalf / 2;
  double slope = _missingValue;
  
  for (int igate = gateNum - nGatesHalf; igate <= gateNum + nGatesHalf; igate++) {

    if (_valid[igate]) {
      double range = _range[igate];
      double phidp = _phidp[igate];
      if (range != _missingValue && phidp != _missingValue) {
        xx[validCount] = range;
        yy[validCount] = phidp;
        validCount++;
        if (gateCount <= nGatesQuarter) {
          countFirstQuarter++;
        }
        if (gateCount >= nGatesHalf + nGatesQuarter) {
          countLastQuarter++;
        }
      }
    }
    
    gateCount++;

  } // igate

  if (validCount < nGatesQuarter) {
    return _missingValue;
  }
  if (countFirstQuarter < 3 || countLastQuarter < 3) {
    return _missingValue;
  }

  ///////////////////////////////////////
  // first try using principal components

  // alloc coords

  double **coords =
    (double **) umalloc2 (validCount, 2, sizeof(double));
  
  // load up coords

  for (int ii = 0; ii < validCount; ii++) {
    coords[ii][0] = xx[ii];
    coords[ii][1] = yy[ii];
  }

  // obtain the principal component transformation for the coord data
  // The technique is applicable here because the first principal
  // component will lie along the axis of maximum variance, which
  // is equivalent to fitting a line through the data points,
  // minimizing the sum of the sguared perpendicular distances
  // from the data to the line.
  
  double means[3];
  double eigenvalues[3];
  double **eigenvectors = (double **) umalloc2(3, 3, sizeof(double));

  if (upct(2, validCount, coords,
           means, eigenvectors, eigenvalues) == 0) {
    double uu = eigenvectors[0][0];
    double vv = eigenvectors[1][0];
    slope = vv / uu;
  }

  ufree2((void **) coords);
  ufree2((void **) eigenvectors);
  
  if (slope != _missingValue) {
    if (slope < -20 || slope > 20) {
      return _missingValue;
    } else {
      return slope;
    }
  }

  //////////////////////////////////////////////////////////////
  // principal components did not work, try regression instead

  // sum up terms

  double sumx = 0.0;
  double sumy = 0.0;
  double sumx2 = 0.0;
  double sumy2 = 0.0;
  double sumxy = 0.0;
  
  for (int ii = 0; ii < validCount; ii++) {
    double xxx = xx[ii];
    double yyy = yy[ii];
    sumx += xxx;
    sumx2 += xxx * xxx;
    sumy += yyy;
    sumy2 += yyy * yyy;
    sumxy += xxx * yyy;
  }

  // compute y-on-x slope

  double num = validCount * sumxy - sumx * sumy;
  double denom = validCount * sumx2 - sumx * sumx;
  double slope_y_on_x;
  
  if (denom != 0.0) {
    slope_y_on_x = num / denom;
  } else {
    slope_y_on_x = 0.0;
  }
  
  // get x-on-y slope

  denom = validCount * sumy2 - sumy * sumy;
  double slope_x_on_y;
  
  if (denom != 0.0) {
    slope_x_on_y = num / denom;
  } else {
    slope_x_on_y = 0.0;
  }

  // average slopes

  if (slope_y_on_x != 0.0 && slope_x_on_y != 0.0) {
    slope = (slope_y_on_x + 1.0 / slope_x_on_y) / 2.0;
  } else if (slope_y_on_x != 0.0) {
    slope = slope_y_on_x;
  } else if (slope_x_on_y != 0.0) {
    slope = 1.0 / slope_x_on_y;
  } else {
    slope = 0.0;
  }

  return 1.0 / slope_x_on_y;

}

/////////////////////////////////////////////////////////////////////////
// This is a Linear Least Square Estimate subroutine to fit a linear
// equation for (xi,yi) (i=1,...,n), so that
//                         yi = a * xi + b
// INPUTs: x(i), y(i), n, (i=1,...,n ).
// OUTPUTs: a ( slope ), b ( intercept ).
//                                                Li Liu   Sep. 23, 92

void KdpCompute::_lse(double &a, double &b, const double *x, const double *y, int n)
  
{

  double xsum = 0.0;
  double ysum = 0.0;
  double xxsum = 0.0;
  double xysum = 0.0;
  double total = n;
  for (int i = 0; i < n; i++) {
    if (x[i] > 1.e35 || y[i] > 1.e35) {
      total--;
    } else {
      xsum += x[i];
      ysum += y[i];
      xxsum += x[i]*x[i];
      xysum += x[i]*y[i];
    }
  }
  double det = total * xxsum - xsum * xsum;
  a = ( total*xysum - xsum*ysum ) / det;
  b = ( ysum*xxsum - xsum*xysum ) / det;

}

//////////////////////////////////////////////////////////////////////////
//  To calculate the mean (ymn) and standard deviation (sd, or,
//  mean square root, msr) of the array y(i) (i=1,...,n).
//                                               Li Liu  Sep. 19, 95

void KdpCompute::_msr(double &ymn, double &sd, const double *y, int n)

{
  
  double ysum  = 0.0;
  double yysum = 0.0;
  double total = n;

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) > 1.e35) {
      total--;
    } else {
      ysum += y[i];
    }
  }

  if (total > 0) {
    ymn = ysum / total;
  } else {
    ymn = 0;
  }

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) < 1.e35) {
      double diff = y[i] - ymn;
      yysum += diff * diff;
    }
  }
  if (total > 0) {
    sd = sqrt(yysum/total);
  } else {
    sd = 0.0;
  }

}

////////////////////
// unfold phidp
// This is Yanting's modified unfold code 

void KdpCompute::_unfoldPhidp(int nGates)

{

  double r_min = 5; /* the minimum range in km depends on radar system
                     * i.e. beyond far field */

  // find first gate without missing data and beyond the far field (5 km)
  
  int firstGate = -1;
  for (int ii = 0; ii < nGates; ii++) {
    if (_phidp[ii] != _missingValue && _range[ii] >= r_min) {
      firstGate = ii;
      break;
    }
  }

  if (firstGate < 0) {
    return;
  }
  
  // compute the beginning phidp by moving up through the gates to find
  // a set of gates over which the stdev of phidp is less than the threshold
  
  double phi_begin = _phidp[firstGate];
  int validCount = 0;
  for (int ii = firstGate; ii < nGates; ii++) {
    
    // compute sdev of phidp over next mgood gates
    
    double phi_mean,sd_phidp;
    _msr(phi_mean, sd_phidp, _phidp + ii, mgood);
    
    // is the standard deviation below the threshold?
    // if so we are ready to begin

    if(sd_phidp < _phidpSdevThreshold) {
      validCount++;
      if(validCount >= mgood) {
        phi_begin = phi_mean;
        firstGate = ii;
        break;
      } else {
        validCount = 0;
      }
    }
    
  } // ii
        
  // for gates before the start gate, or after the last gate,
  // fill in missing regions with the surounding gates

  for (int ii = -EXTRA_HALF; ii < 0; ii++) {
    _phidp[ii] = phi_begin;
  }
  
  for (int ii = 0; ii < firstGate; ii++) {
    if (_phidp[ii] == _missingValue) {
      _phidp[ii] = phi_begin;
    }
  }
  
  for (int ii = firstGate; ii < nGates; ii++) {
    if (_phidp[ii] == _missingValue) {
      _phidp[ii] = _phidp[ii-1];
    }
  }
  
  // look for fold

  double phi_recent = phi_begin;
  for (int ii = firstGate; ii < nGates; ii++) {
    
    // compute mean and sdev
    
    double phi_mean, sd_phidp;
    _msr(phi_mean, sd_phidp, _phidp + ii, mgood);
    
    if(sd_phidp < _phidpSdevThreshold) {
      phi_recent = phi_mean;
    }
    
    if (phi_recent - _phidp[ii] > _phidpFoldRange / 3) {
      _phidp[ii] += _phidpFoldRange;
    }

  } // ii

  for (int ii = nGates; ii < nGates +  EXTRA_HALF; ii++) {
    _phidp[ii] = _phidp[nGates-1];
  }

}
    
