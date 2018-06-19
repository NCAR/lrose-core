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

#include <cassert>
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
  _clearStats();
  clearValues();

  _histMin = NAN;
  _histMax = NAN;
  _histDelta = NAN;

  _histNBins = 60;
  _histNSdev = 3.0;

}

//////////////////////////////////////////////////////////////////
// destructor

Distribution::~Distribution()

{

}

//////////////////////////////////////////////////////////////////
// clear the stats

void Distribution::_clearStats()

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

  _pdfAvail = false;
  _pdfMedian = NAN;
  _pdfMode = NAN;

  _clearHist();

}

//////////////////////////////////////////////////////////////////
// clear histogram

void Distribution::_clearHist()

{

  _histX.clear();

  _histCount.clear();
  _histDensity.clear();

  _histCumCount.clear();
  _histCumDensity.clear();

  _histPdf.clear();
  _histCdf.clear();

  _histMedian = NAN;
  _histMode = NAN;

}

//////////////////////////////////////////////////////////////////
// add a data value

void Distribution::addValue(double xx) 
{

  _values.push_back(xx);
  _nVals = _values.size();

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
    _clearStats();
    _values.clear();
    _nVals = _values.size();
    return;
  }

  _values = vals;
  _nVals = _values.size();
  _min = _values[0];
  _max = _values[0];

  for (size_t ii = 1; ii < _nVals; ii++) {
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
  _nVals = _values.size();
  _clearStats();
}

////////////////////////////////////////////////////
// compute and return the mean

double Distribution::computeMean()
  
{

  if (_nVals < 1) {
    _mean = NAN;
    return _mean;
  }

  double nn = _nVals;
  double sumx = 0.0;
  
  for (size_t ii = 0; ii < _nVals; ii++) {
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

  if (_nVals < 2) {
    _sdev = NAN;
    return _sdev;
  }

  double nn = _nVals;
  double sumx = 0.0;
  double sumx2 = 0.0;
  
  for (size_t ii = 0; ii < _nVals; ii++) {
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

  double nn = _nVals;
  double sum = 0.0;
  for (size_t ii = 0; ii < _nVals; ii++) {
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

  double nn = _nVals;
  double sum = 0.0;
  for (size_t ii = 0; ii < _nVals; ii++) {
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

  double nn = _nVals;
  double sumSkew = 0.0;
  double sumKurt = 0.0;
  for (size_t ii = 0; ii < _nVals; ii++) {
    double xx = _values[ii] - _mean;
    sumSkew += pow(xx, 3.0);
    sumKurt += pow(xx, 4.0);
  }

  _skewness = (sumSkew / nn) / pow(_sdev, 3.0);
  _kurtosis = ((sumKurt / nn) / pow(_sdev, 4.0)) - 3.0;

}

//////////////////////////////////////////////////////////////////
// set histogram range
// the min val is the mid val of the first bin
// the max val is the mid val of the last bin
  
void Distribution::setHistRange(double minVal, double maxVal)

{

  _histMin = minVal;
  _histMax = maxVal;

  if (_histNBins < 2) {
    _histNBins = 60;
  }
  _histDelta = (_histMax - _histMin) / (double) _histNBins;

}
  
//////////////////////////////////////////////////////////////////
// set histogram range to some multiple of the
// standard deviation of the data

void Distribution::setHistRangeFromSdev(double nSdev)

{

  _histNSdev = nSdev;

  // compute mean and standard deviation
  
  if (std::isnan(_sdev)) {
    computeSdev();
  }

  _histMin = _mean - _histNSdev * _sdev;
  _histMax = _mean + _histNSdev * _sdev;

  if (_histNBins < 2) {
    _histNBins = 60;
  }
  _histDelta = (_histMax - _histMin) / (double) _histNBins;

}
  
//////////////////////////////////////////////////////////////////
// compute histogram, based on current settings
// if hist min and max have not been set, they are computed
// from the data

void Distribution::computeHistogram()

{

  _clearHist();

  // if limits have not been set, compute them from the data limits

  if (std::isnan(_histMin) || std::isnan(_histMax)) {
    setHistRange(_min, _max);
  }

  // set counts

  for (size_t jj = 0; jj < _histNBins; jj++) {
    _histCount.push_back(0.0);
    _histX.push_back(_histMin + jj * _histDelta);
  } // jj
  
  for (size_t ii = 0; ii < _nVals; ii++) {
    double val = _values[ii];
    int index = (int) ((val - _histMin) / _histDelta + 0.5);
    if (index >= 0 && index < (int) _histNBins) {
      _histCount[index]++;
    }
  } // ii

  // compute density

  for (size_t jj = 0; jj < _histNBins; jj++) {
    _histDensity.push_back((_histCount[jj] / (double) _nVals) / _histDelta);
  } // jj
  
  if (_pdfAvail) {
    for (size_t jj = 0; jj < _histNBins; jj++) {
      _histPdf.push_back(getPdf(_histX[jj]));
    } // jj
    computeHistCdf();
  }

  // cumulative counts and density

  double sumCount = 0.0;
  double sumDensity = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    sumCount += _histCount[jj];
    sumDensity += _histDensity[jj] * _histDelta;
    _histCumCount.push_back(sumCount);
    _histCumDensity.push_back(sumDensity);
  }
  double corr = 1.0 / sumDensity;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    _histCumDensity[jj] *= corr;
  }

  // find the median from the histogram

  double nValsHalf = (double) _nVals / 2.0;
  for (size_t jj = 1; jj < _histNBins; jj++) {
    if (_histCumCount[jj-1] <= nValsHalf && _histCumCount[jj] >= nValsHalf) {
      double delta = _histCumCount[jj] - _histCumCount[jj-1];
      double frac = (nValsHalf - _histCumCount[jj-1]) / delta;
      _histMedian = _histX[jj-1] + frac * (_histX[jj] - _histX[jj-1]);
      break;
    }
  }

  // find the mode from the histogram

  double maxCount = -1.0e99;
  int modeIndex = 0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    if (_histCount[jj] > maxCount) {
      maxCount = _histCount[jj];
      modeIndex = jj;
    }
  } // jj
  _histMode = _histX[modeIndex];

}

//////////////////////////////////////////////////////////////////
// print histogram as text

void Distribution::printHistogram(FILE *out)

{

  if (_nVals < 2 || _histNBins < 2) {
    return;
  }

  computeGof();

  fprintf(out, "======================= Histogram ===========================\n");
  fprintf(out, "  nValues: %d\n", (int) getNValues());
  fprintf(out, "  histSize: %d\n", (int) _histNBins);
  fprintf(out, "  histDelta: %g\n", _histDelta);
  fprintf(out, "  histMin: %g\n", _histMin);
  fprintf(out, "  histMax: %g\n", _histMax);
  fprintf(out, "  histMedian: %g\n", _histMedian);
  fprintf(out, "  histMode: %g\n", _histMode);
  fprintf(out, "  pdfMedian: %g\n", _pdfMedian);
  fprintf(out, "  pdfMode: %g\n", _pdfMode);
  fprintf(out, "  rmsePdf, gof: %6.3f, %6.3f\n", _rmsePdf, _gof);
  fprintf(out, "  smk, smk95: %6.3f, %6.3f\n", _smk, _smk95);
  fprintf(out, "\n");
  fprintf(out, "%4s %8s %8s %6s %6s %6s %6s \n",
          "bin", "xx", "count", "hpdf", "hcdf", "pdf", "cdf");
  
  double maxDensity = 0;
  for (size_t jj = 0; jj < _histDensity.size(); jj++) {
    if (_histDensity[jj] > maxDensity) {
      maxDensity = _histDensity[jj];
    }
  } // jj

  for (size_t jj = 0; jj < _histNBins; jj++) {

    int count = _histCount[jj];
    double xx = _histX[jj];
    double pdf = getPdf(xx);
    double cdf = -9999;
    if (_histCdf.size() == _histNBins) {
      cdf = _histCdf[jj];
    }
    double hpdf = _histDensity[jj];
    double hcdf = _histCumDensity[jj];
    
    fprintf(out, "%4d %8.3f %8d %6.3f %6.3f %6.3f %6.3f %6.3f ",
            (int) jj, xx, count, hpdf, hcdf, pdf, cdf, fabs(hcdf - cdf));
    
    int ipdf = (int) ((pdf / maxDensity) * 60.0);
    int nStars = (int) ((_histDensity[jj] / maxDensity) * 60.0);
    for (int ii = 0; ii < nStars; ii++) {
      if (ii == ipdf) {
        fprintf(out, "*");
      } else {
        fprintf(out, ":");
      }
    }
    if (ipdf >= nStars) {
      for (int ii = nStars; ii < ipdf; ii++) {
        fprintf(out, " ");
      }
      fprintf(out, "*");
    }
    fprintf(out, "\n");

  }
  fprintf(out, "===============================================================\n");
  
}

//////////////////////////////////////////////////////////////////
// compute the histogram-based CDF
// assumes the histogram and fit have been computed

void Distribution::computeHistCdf()

{

  _histCdf.clear();
  double sum = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    double prob = _histPdf[jj] * _histDelta;
    sum += prob;
    _histCdf.push_back(sum);
  }

  double correction = 1.0 / sum;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    _histCdf[jj] *= correction;
  }

  // find the median from the PDF

  for (size_t jj = 1; jj < _histNBins; jj++) {
    if (_histCdf[jj-1] <= 0.5 && _histCdf[jj] >= 0.5) {
      double delta = _histCdf[jj] - _histCdf[jj-1];
      double frac = (0.5 - _histCdf[jj-1]) / delta;
      _pdfMedian = _histX[jj-1] + frac * (_histX[jj] - _histX[jj-1]);
      break;
    }
  }

  // find the mode from the PDF

  double maxPdf = -1.0e99;
  int modeIndex = 0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    if (_histPdf[jj] > maxPdf) {
      maxPdf = _histPdf[jj];
      modeIndex = jj;
    }
  } // jj
  _pdfMode = _histX[modeIndex];

}

//////////////////////////////////////////////////////////////////
// compute goodness of fit test
// kk is number of intervals used in test
// assumes histogram and fit have been computed

void Distribution::computeGof(size_t nIntervals)
  
{

  if (_nVals < 1) {
    if (_verbose) {
      cerr << "ERROR - Distribution::computeGof()" << endl;
      cerr << "  No values in histogram" << endl;
    }
    return;
  }

  if (_histNBins < 1) {
    if (_verbose) {
      cerr << "ERROR - Distribution::computeGof()" << endl;
      cerr << "  Histogram has not been computed" << endl;
    }
    return;
  }

  if (_histPdf.size() != _histNBins) {
    if (_verbose) {
      cerr << "ERROR - Distribution::computeGof()" << endl;
      cerr << "  PDF fit has not been performed" << endl;
      cerr << "  _histPdf.size(): " << _histPdf.size() << endl;
      cerr << "  _histNBins: " << _histNBins << endl;
    }
    return;
  }

  double nn = _nVals;
  double intervalProb = 1.0 / (double) nIntervals;
  
  if (_debug) {
    cerr << "====>> DistNormal::computeGof <<====" << endl;
    cerr << "  nIntervals: " << nIntervals << endl;
  }

  double sumHistProb = 0.0;
  double sumGof = 0.0;
  double sumErrSq = 0.0;
  double nGof = 0.0;
  double sumPdfProb = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    sumHistProb += _histCount[jj] / nn;
    double xx = _histMin + jj * _histDelta;
    double pdfProb = getPdf(xx) * _histDelta;
    sumPdfProb += pdfProb;
    if ((sumHistProb > intervalProb) || (jj == _histNBins - 1)) {
      double error = sumHistProb - sumPdfProb;
      sumErrSq += (error * error);
      double gofFac = (error * error) / sumPdfProb;
      sumGof += gofFac;
      nGof++;
      sumHistProb = 0.0;
      sumPdfProb = 0.0;
    }
  } // jj
  _gof = sumGof;

  // compute rmse for pdf

  double sumSqErr = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    double pdfDens = _histPdf[jj];
    double histDens = _histDensity[jj];
    double error = pdfDens - histDens;
    sumSqErr += error * error;
  }
  _rmsePdf = sqrt(sumSqErr / _histNBins);

  // compute smirnov kolmogorov statistic from the CDFs

  double maxCdfDiff = 0.0;
  for (size_t jj = 0; jj < _histNBins; jj++) {
    double cdfDiff = fabs(_histCdf[jj] - _histCumDensity[jj]);
    if (cdfDiff > maxCdfDiff) {
      maxCdfDiff = cdfDiff;
    }
  } // jj

  _smk = maxCdfDiff;
  _smk95 = 1.36 / sqrt(nn);

  if (_debug) {
    cerr << "==> rmsePdf: " << _rmsePdf << endl;
    cerr << "==> gof: " << _gof << endl;
    cerr << "==> smk: " << _smk << endl;
    cerr << "==> smk95: " << _smk95 << endl;
  }

}

