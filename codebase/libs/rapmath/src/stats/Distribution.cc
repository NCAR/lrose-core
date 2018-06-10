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
// Distribution.cc
//
// Statistical distributions, base class
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <rapmath/Distribution.hh>
using namespace std;

const double Distribution::sqrt2 = sqrt(2.0);
const double Distribution::sqrt2Pi = sqrt(2.0 * M_PI);

////////////////////////////////////////////////////
// Base class

Distribution::Distribution()
  
{

  _debug = false;
  _verbose = false;
  _initStats();

}

//////////////////////////////////////////////////////////////////
// destructor

Distribution::~Distribution()

{

}

//////////////////////////////////////////////////////////////////
// initialize the stats

void Distribution::_initStats()

{

  _min = NAN;
  _max = NAN;
  _mean = NAN;
  _median = NAN;
  _mode = NAN;
  _sdev = NAN;
  _variance = NAN;
  _skewness = NAN;
  _kurtosis = NAN;

  _hist.clear();
  _histMin = NAN;
  _histDelta = NAN;

}

//////////////////////////////////////////////////////////////////
// add a data value

void Distribution::addValue(double xx) 
{
  _values.push_back(xx);
  if (std::isnan(_min)) {
    _min = xx;
  } else if (!std::isnan(xx) && xx < _min) {
    _min = xx;
  }
  if (std::isnan(_max)) {
    _max = xx;
  } else if (!std::isnan(xx) && xx > _max) {
    _max = xx;
  }
}

//////////////////////////////////////////////////////////////////
// set the data values
  
void Distribution::setValues(const vector<double> &vals)
{
  if (vals.size() < 1) {
    _initStats();
    _values.clear();
    return;
  }
  _min = _values[0];
  _max = _values[0];
  for (size_t ii = 1; ii < _values.size(); ii++) {
    double xx = _values[ii];
    if (!std::isnan(xx) && xx < _min) {
      _min = xx;
    }
    if (!std::isnan(xx) && xx > _max) {
      _max = xx;
    }
  }
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void Distribution::clearValues()
{
  _values.clear();
  _initStats();
}

////////////////////////////////////////////////////
// compute and return the mean

double Distribution::computeMean()
  
{

  if (_values.size() < 1) {
    _mean = NAN;
    return _mean;
  }

  double nn = _values.size();
  double sumx = 0.0;
  
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double xx = _values[ii];
    sumx += xx;
  }

  _mean = sumx / nn;

  return _mean;

}

////////////////////////////////////////////////////
// compute and return the standard deviation
// side effects: also computes the mean and variance

double Distribution::computeSdev()
  
{

  if (_values.size() < 2) {
    _sdev = NAN;
    return _sdev;
  }

  double nn = _values.size();
  double sumx = 0.0;
  double sumx2 = 0.0;
  
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double xx = _values[ii];
    sumx += xx;
    sumx2 += xx * xx;
  }

  _mean = sumx / nn;
  _variance = (sumx2 - (sumx * sumx) / nn) / nn;
  
  if (_variance >= 0.0) {
    _sdev = sqrt(_variance);
  } else {
    _sdev = 0.0;
  }

  return _sdev;

}

////////////////////////////////////////////////////
// compute and return the skewness
// side effects: also computes the mean, sdev and variance

double Distribution::computeSkewness()
  
{

  computeSdev();

  double nn = _values.size();
  double sum = 0.0;
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double xx = _values[ii] - _mean;
    sum += pow(xx, 3.0);
  }

  _skewness = (sum / nn) / pow(_sdev, 3.0);

  return _skewness;

}

////////////////////////////////////////////////////
// compute and return the kurtosis
// side effects: also computes the mean, sdev and variance

double Distribution::computeKurtosis()
  
{

  computeSdev();

  double nn = _values.size();
  double sum = 0.0;
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double xx = _values[ii] - _mean;
    sum += pow(xx, 4.0);
  }

  _kurtosis = ((sum / nn) / pow(_sdev, 4.0)) - 3.0;

  return _kurtosis;

}

//////////////////////////////////////////////////////////////////
// compute the basic stats
// mean, sdev, variance, skewness and kurtosis

void Distribution::computeBasicStats()

{

  computeSdev();

  double nn = _values.size();
  double sumSkew = 0.0;
  double sumKurt = 0.0;
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double xx = _values[ii] - _mean;
    sumSkew += pow(xx, 3.0);
    sumKurt += pow(xx, 4.0);
  }

  _skewness = (sumSkew / nn) / pow(_sdev, 3.0);
  _kurtosis = ((sumKurt / nn) / pow(_sdev, 4.0)) - 3.0;

}

//////////////////////////////////////////////////////////////////
// compute histogram
// if n is not specified, the default is used
// if histMin is not specified, the min in the data is used
// if histDelta is not specified, it is computed from data range
// histMin is the value at lower edge of the first histogram bin
// histDelta is the delta between the center of adjacent bins

void Distribution::computeHistogram(size_t n /* = 60 */,
                                    double histMin /* = NAN */,
                                    double histDelta /* = NAN */)

{

  if (std::isnan(histMin)) {
    _histMin = _min;
  } else {
    _histMin = histMin;
  }

  if (std::isnan(histDelta)) {
    // compute histDelta to ensure we include
    // both min and max values
    _histDelta = ((_max - _min) / (double) n) * 1.0001;
    _histMin -= _histDelta * 0.0001;
  } else {
    _histDelta = histDelta;
  }

  _hist.clear();
  for (size_t jj = 0; jj < n; jj++) {
    _hist.push_back(0.0);
  } // jj
  
  for (size_t ii = 0; ii < _values.size(); ii++) {
    double val = _values[ii];
    int index = (int) ((val - _histMin) / _histDelta);
    if (index >= 0 && index < (int) _hist.size()) {
      _hist[index]++;
    }
  } // ii

  if (_verbose) {
    printHistogram(stderr);
  }

}

//////////////////////////////////////////////////////////////////
// print histogram as text

void Distribution::printHistogram(FILE *out)

{

  if (_hist.size() < 2) {
    return;
  }

  fprintf(out, "======================= Histogram ===========================\n");
  fprintf(out, "  histSize: %d\n", (int) _hist.size());
  fprintf(out, "  histDelta: %g\n", _histDelta);
  fprintf(out, "  histMin: %g\n", _histMin);
  fprintf(out, "  histMax: %g\n", _histMin + _hist.size() * _histDelta);
  fprintf(out, "\n");
  fprintf(out, "%8s %8s %8s %8s\n", "bin", "min", "max", "count");
  
  double maxCount = 0;
  for (size_t jj = 0; jj < _hist.size(); jj++) {
    if (_hist[jj] > maxCount) {
      maxCount = _hist[jj];
    }
  } // jj

  for (size_t jj = 0; jj < _hist.size(); jj++) {

    double binMin = _histMin + jj * _histDelta;
    double binMax = _histMin + (jj+1) * _histDelta;
    int binCount = _hist[jj];
    
    fprintf(out, "%8d %8.3f %8.3f %8d  ",
            (int) jj, binMin, binMax, binCount);
    
    int nStars = (int) ((binCount / maxCount) * 40.0);
    for (int ii = 0; ii < nStars; ii++) {
      fprintf(out, "*");
    }
    fprintf(out, "\n");

  }
  fprintf(out, "===============================================================\n");
  
}

