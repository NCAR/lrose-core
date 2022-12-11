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
// Stats.cc
//
// Statistical distribution - copied from libs/rapmath and modified
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdio>
#include <iostream>
#include "Stats.hh"
using namespace std;

////////////////////////////////////////////////////
// Base class

Stats::Stats()
  
{
  
  _initDone = false;
  clear();

}

//////////////////////////////////////////////////////////////////
// destructor

Stats::~Stats()

{

}

//////////////////////////////////////////////////////////////////
// clear

void Stats::clear()

{
  
  _sum = 0.0;
  _sumSq = 0.0;
  
  _min = NAN;
  _max = NAN;
  _mean = NAN;
  _median = NAN;
  _mode = NAN;
  _sdev = NAN;
  _variance = NAN;
  _skewness = NAN;
  _kurtosis = NAN;
  
  for (size_t ii = 0; ii < _histCount.size(); ii++) {
    _histCount[ii] = 0.0;
  } // ii

}

//////////////////////////////////////////////////////////////////
// initialize histogram range
// the min val is the mid val of the first bin
// the max val is the mid val of the last bin
  
void Stats::init(double minVal, double maxVal, size_t nBins)

{

  if (nBins < 3) {
    cerr << "Stats::init, nBins too low: " << nBins << endl;
    assert(nBins >= 3);
  }
  _histNBins = nBins;

  if (minVal >= maxVal) {
    cerr << "Stats::init, maxVal must exceed minVal" << endl;
    cerr << "  minVal, maxVal: " << minVal << ", " << maxVal << endl;
    assert(maxVal > minVal);
  }
  
  _histMin = minVal;
  _histMax = maxVal;

  _histDelta = (_histMax - _histMin) / (double) _histNBins;
  
  // set counts

  _histX.clear();
  _histCount.clear();
  
  for (size_t ii = 0; ii < _histNBins; ii++) {
    _histX.push_back(_histMin + ii * _histDelta);
    _histCount.push_back(0.0);
  } // ii

  _initDone = true;
  
}
  
//////////////////////////////////////////////////////////////////
// add a data value

void Stats::addValue(double val) 
{

  assert(_initDone);
  
  int index = (int) ((val - _histMin) / _histDelta + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > (int) (_histNBins - 1)) {
    index = _histNBins - 1;
  }

  _histCount[index]++;
  _nVals++;

  _sum += val;
  _sumSq += val * val;
  
  if (std::isnan(_min)) {
    _min = val;
  } else if (!std::isnan(val) && val < _min) {
    _min = val;
  }

  if (std::isnan(_max)) {
    _max = val;
  } else if (!std::isnan(val) && val > _max) {
    _max = val;
  }

}

//////////////////////////////////////////////////////////////////
// set the data values
  
void Stats::setValues(const vector<double> &vals)
{
  for (size_t ii = 1; ii < vals.size(); ii++) {
    addValue(vals[ii]);
  }
}

////////////////////////////////////////////////////
// compute the statistics

void Stats::computeStats()
  
{

  if (_nVals < 1) {
    return;
  }

  // compute mean
  
  _mean = _sum / _nVals;

  if (_nVals < 2) {
    return;
  }

  // compute variance and sdev
  
  _variance = (_sumSq - (_sum * _sum) / _nVals) / (_nVals - 1.0);
  
  if (_variance >= 0.0) {
    _sdev = sqrt(_variance);
  } else {
    _sdev = 0.0;
  }

  // skewness and kurtosis
  
  double sum3 = 0.0;
  double sum4 = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    double xx = _histX[jj] - _mean;
    sum3 += (pow(xx, 3.0) * _histCount[jj]);
    sum3 += (pow(xx, 4.0) * _histCount[jj]);
  }

  _skewness = ((sum3 / _nVals) / pow(_sdev, 3.0));
  _kurtosis = ((sum4 / _nVals) / pow(_sdev, 4.0)) - 3.0;
  
  // find the median from the histogram
  
  double nValsHalf = (double) _nVals / 2.0;
  double cumCount = _histCount[0];
  for (size_t jj = 1; jj < _histNBins; jj++) {
    double nextCount = cumCount + _histCount[jj];
    if (cumCount <= nValsHalf && nextCount >= nValsHalf) {
      double delta = _histCount[jj];
      double frac = (nValsHalf - cumCount) / delta;
      _median = _histX[jj-1] + frac * (_histX[jj] - _histX[jj-1]);
      break;
    }
    cumCount = nextCount;
  }

  // find the mode from the histogram

  double maxCount = -1.0e99;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    if (_histCount[jj] > maxCount) {
      maxCount = _histCount[jj];
      _mode = _histX[jj];
    }
  } // jj

}

//////////////////////////////////////////////////////////////////
// print stats

void Stats::printStats(FILE *out)
  
{
  
  fprintf(out, "  nValues: %g\n", _nVals);
  fprintf(out, "  min: %g\n", _min);
  fprintf(out, "  max: %g\n", _max);
  fprintf(out, "  mean: %g\n", _mean);
  fprintf(out, "  sdev: %g\n", _sdev);
  fprintf(out, "  skewness: %g\n", _skewness);
  fprintf(out, "  kurtosis: %g\n", _kurtosis);
  fprintf(out, "  mode: %g\n", _mode);
  fprintf(out, "  median: %g\n", _median);
  fprintf(out, "\n");

}

//////////////////////////////////////////////////////////////////
// print histogram as text

void Stats::printHistogram(FILE *out)

{

  printStats(out);

  if (_nVals < 2 || _histNBins < 2) {
    return;
  }

  fprintf(out,
          "======================= Histogram ===========================\n");
  fprintf(out, "%4s %8s %8s\n", "bin", "xx", "count");
  
  for (size_t jj = 0; jj < _histNBins; jj++) {

    int count = _histCount[jj];
    double xx = _histX[jj];
    
    fprintf(out, "%4d %8.3f %8d\n", (int) jj, xx, count);

  } // jj
  
}

