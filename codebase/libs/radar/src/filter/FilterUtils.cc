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
////////////////////////////////////////////////////////////////////////
// FilterUtils
//
// Filter utililties for radar data
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <iostream>
#include <radar/FilterUtils.hh>
#include <toolsa/TaArray.hh>

using namespace std;

////////////////////////////////////////////////////
// apply a median filter to a double field

void FilterUtils::applyMedianFilter(double *field,
                                    int fieldLen,
                                    int filterLen)
  
{
  
  // make sure filter len is odd

  int halfFilt = filterLen / 2;
  int len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }

  // make copy

  TaArray<double> buf_, copy_;
  double *buf = buf_.alloc(len);
  double *copy = copy_.alloc(fieldLen);
  memcpy(copy, field, fieldLen * sizeof(double));
  
  for (int ii = halfFilt; ii < fieldLen - halfFilt; ii++) {
    
    memcpy(buf, copy + ii - halfFilt, len * sizeof(double));
    qsort(buf, len, sizeof(double), _doubleCompare);
    field[ii] = buf[halfFilt];

  }

}

////////////////////////////////////////////////////
// apply a median filter to a double field
// handle missing values

void FilterUtils::applyMedianFilter(double *field,
                                    int fieldLen,
                                    int filterLen,
                                    double missingVal)
  
{
  
  // make sure filter len is odd

  int halfFilt = filterLen / 2;
  int len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }

  // make copy

  TaArray<double> copy_;
  double *copy = copy_.alloc(fieldLen);
  memcpy(copy, field, fieldLen * sizeof(double));
  
  for (int ii = halfFilt; ii < fieldLen - halfFilt; ii++) {

    vector<double> vals;
    for (int jj = ii - halfFilt; jj <= ii + halfFilt; jj++) {
      if (copy[jj] != missingVal) {
        vals.push_back(copy[jj]);
      }
    } // jj

    if (vals.size() > 0) {
      sort(vals.begin(), vals.end());
      field[ii] = vals[vals.size() / 2];
    } else {
      field[ii] = missingVal;
    }
    
  } // ii

}

////////////////////////////////////////////////////
// apply a median filter to an integer field

void FilterUtils::applyMedianFilter(int *field,
                                    int fieldLen,
                                    int filterLen)
  
{
  
  // make sure filter len is odd

  int halfFilt = filterLen / 2;
  int len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }
  
  // make copy

  TaArray<int> buf_, copy_;
  int *buf = buf_.alloc(len);
  int *copy = copy_.alloc(fieldLen);
  memcpy(copy, field, fieldLen * sizeof(int));

  // apply filter

  for (int ii = halfFilt; ii < fieldLen - halfFilt; ii++) {
    
    memcpy(buf, copy + ii - halfFilt, len * sizeof(int));
    qsort(buf, len, sizeof(int), _intCompare);
    field[ii] = buf[halfFilt];

  }

}

////////////////////////////////////////////////////
// apply a median filter to a int field
// handle missing values

void FilterUtils::applyMedianFilter(int *field,
                                    int fieldLen,
                                    int filterLen,
                                    int missingVal)
  
{
  
  // make sure filter len is odd

  int halfFilt = filterLen / 2;
  int len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }

  // make copy

  TaArray<int> copy_;
  int *copy = copy_.alloc(fieldLen);
  memcpy(copy, field, fieldLen * sizeof(int));
  
  for (int ii = halfFilt; ii < fieldLen - halfFilt; ii++) {

    vector<int> vals;
    for (int jj = ii - halfFilt; jj <= ii + halfFilt; jj++) {
      if (copy[jj] != missingVal) {
        vals.push_back(copy[jj]);
      }
    } // jj

    if (vals.size() > 0) {
      sort(vals.begin(), vals.end());
      field[ii] = vals[vals.size() / 2];
    } else {
      field[ii] = missingVal;
    }
    
  } // ii

}

/////////////////////////////////////////////////////
// define functions to be used for sorting

int FilterUtils::_doubleCompare(const void *i, const void *j)
{
  double *f1 = (double *) i;
  double *f2 = (double *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

int FilterUtils::_intCompare(const void *i, const void *j)
{
  int *f1 = (int *) i;
  int *f2 = (int *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

/////////////////////////////////////////////////////
// interpolate linearly between points
// return yVal for given xVal

double FilterUtils::linearInterp(double xx1, double yy1,
                                 double xx2, double yy2,
                                 double xVal)

{

  double slope = (yy2 - yy1) / (xx2 - xx1);
  double xOffset = xVal - xx1;
  double yOffset = xOffset * slope;
  return yy1 + yOffset;

}

/////////////////////////////////////////////////////////////////
// compute standard deviation of a field, over a kernel in range
//
// Set field values to missingVal if they are missing.
// The sdev will be set to missingVal if not enough data is
// available for computing the standard deviation.

void FilterUtils::computeSdevInRange(const double *field,
                                     double *sdev,
                                     int nGates,
                                     int nGatesKernel,
                                     double missingVal)
  
{
  
  int nGatesHalf = nGatesKernel / 2;
  
  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // sdev computed in range
  
  for (int igate = 0; igate < nGates; igate++) {

    // initialize
    
    sdev[igate] = missingVal;

    // compute sums etc. for stats over the kernel space
    
    double nVal = 0.0;
    double sumVal = 0.0;
    double sumValSq = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      
      double zz = field[jgate];
      if (zz != missingVal) {
        sumVal += zz;
        sumValSq += (zz * zz);
        nVal++;
      }
      
    } // jgate
    
    if (nVal > 0) {
      double meanVal = sumVal / nVal;
      if (nVal > 2) {
        double term1 = sumValSq / nVal;
        double term2 = meanVal * meanVal;
        if (term1 >= term2) {
          sdev[igate] = sqrt(term1 - term2);
        }
      }
    }
    
  } // igate

}

/////////////////////////////////////////////////////////////////
// compute trend deviation of a field, over a kernel in range
//
// Set field values to missingVal if they are missing.
// The trend dev will be set to missingVal if not enough data is
// available for computing the result.
//
// If mean is not NULL, the mean values are stored there.
// If texture is not NULL, the texture values are stored there.
//
// The trend rms deviation differs from standard deviation.
// It is computed as follows:
//  (a) compute kernel mean at each point
//  (b) compute residual of data from trend mean
//  (c) compute root mean square of residual over kernel.

void FilterUtils::computeTrendDevInRange(const double *field,
                                         int nGates,
                                         int nGatesKernel,
                                         double missingVal,
                                         double *mean /* = NULL */,
                                         double *texture /* = NULL */)
  
{

  // ensure we have odd nmber of gates in the kernel

  int nGatesHalf = nGatesKernel / 2;
  if (nGatesHalf < 1) {
    nGatesHalf = 1;
  }
  nGatesKernel = nGatesHalf * 2 + 1;
  
  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate

  // kernel means and residuals
  
  TaArray<double> residual_;
  double *residual = residual_.alloc(nGates);

  for (int igate = 0; igate < nGates; igate++) {
    
    // compute sums over the kernel space
    
    int nVal = 0;
    double sumVal = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      double val = field[jgate];
      if (val != missingVal) {
        sumVal += val;
        nVal++;
      }
    } // jgate
    
    if (nVal > 0 && field[igate] != missingVal) {
      double meanVal = sumVal / (double) nVal;
      residual[igate] = meanVal - field[igate];
      if (mean != NULL) {
        mean[igate] = meanVal;
      }
    } else {
      residual[igate] = missingVal;
    }
    
  } // igate
  
  // trend deviation is root mean square of residuals
  
  for (int igate = 0; igate < nGates; igate++) {
    
    // compute sums sq for stats over the kernel
    
    int nVal = 0;
    double sumValSq = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      double val = residual[jgate];
      if (val != missingVal) {
        sumValSq += val * val;
        nVal++;
      }
    } // jgate
    
    if (texture != NULL) {
      if (nVal > 0) {
        double rms = sqrt(sumValSq / (double) nVal);
        texture[igate] = rms;
      } else {
        texture[igate] = missingVal;
      }
    }
    
  } // igate

}

