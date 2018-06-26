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
// Distribution.hh
//
// Statistical distributions
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#ifndef Distribution_hh
#define Distribution_hh

#include <string>
#include <vector>
#include <rapmath/stats.h>
#include <cmath>
#include <cstdio>
using namespace std;

////////////////////////
// Base class
//
// Missing or unset values are set to NAN

class Distribution {
  
public:
  
  // constructor
  
  Distribution();
  
  // destructor
  
  virtual ~Distribution();
  
  // debugging
  
  void setDebug(bool state) { _debug = state; }
  void setVerbose(bool state) {
    _verbose = state;
    if (state) setDebug(true);
  }

  // add a data value
  
  void addValue(double xx);

  // set the data values
  
  void setValues(const vector<double> &vals);

  // set number of bins in histogram
  
  void setHistNBins(size_t val) { _histNBins = val; }

  // set histogram range
  // the min val is the mid val of the first bin
  // the max val is the mid val of the last bin
  
  void setHistRange(double minVal, double maxVal);
  
  // set histogram range to some multiple of the
  // standard deviation of the data
  
  void setHistRangeFromSdev(double nSdev);
  
  // clear the data value

  void clearValues();

  // compute and return the mean

  double computeMean();

  // compute and return the standard deviation
  // side effects: also computes the mean and variance

  double computeSdev();

  // compute and return the skewness
  // side effects: also computes the mean, sdev and variance

  double computeSkewness();

  // compute and return the kurtosis
  // side effects: also computes the mean, sdev and variance

  double computeKurtosis();

  // compute the basic stats:
  // mean, sdev, variance, skewness and kurtosis

  void computeBasicStats();
  
  // compute histogram, based on current settings
  // if hist min and max have not been set, they are computed
  // from the data
  
  void computeHistogram();
  
  // perform a fit - abstract base class
  // must be overridden in derived class
  // values must have been set
  // returns 0 on success, -1 on failure
  
  virtual int performFit() = 0;

  // compute the histogram-based CDF
  // assumes the histogram and fit have been computed
  
  void computeHistCdf();

  // get the pdf for a given x
  // must be overridden in derived class

  virtual double getPdf(double xx) = 0;

  // get the cdf for a given x
  // must be overridden in derived class

  virtual double getCdf(double xx) = 0;

  // compute goodness of fit test
  // kk is number of intervals used in test
  
  void computeGof(size_t kk = 11);

  // get methods

  size_t getNValues() const { return _values.size(); }
  const vector<double> &getValues() const { return _values; }

  double getMin() const { return _min; }
  double getMax() const { return _max; }
  double getMean() const { return _mean; }
  double getSdev() const { return _sdev; }
  double getVariance() const { return _variance; }
  double getSkewness() const { return _skewness; }
  double getKurtosis() const { return _kurtosis; }

  size_t getHistNBins() const { return _histNBins; }
  double getHistMin() const { return _histMin; }
  double getHistMax() const { return _histMax; }
  double getHistDelta() const { return _histDelta; }

  const vector<double> &getHistX() const { return _histX; }
  const vector<double> &getHistCount() const { return _histCount; }
  const vector<double> &getHistDensity() const { return _histDensity; }

  double getHistMedian() const { return _histMedian; }
  double getHistMode() const { return _histMode; }

  double getRmsePdf() const { return _rmsePdf; }
  double getGof() const { return _gof; }
  double getSmk() const { return _smk; }
  double getSmk95() const { return _smk95; }

  bool getPdfAvail() const { return _pdfAvail; }
  const vector<double> &getHistPdf() const { return _histPdf; }
  const vector<double> &getHistCdf() const { return _histCdf; }

  double getPdfMedian() const { return _pdfMedian; }
  double getPdfMode() const { return _pdfMode; }

  const vector<double> &getHistCumCount() const { return _histCumCount; }
  const vector<double> &getHistCumDensity() const { return _histCumDensity; }

  // print histogram as text
  
  void printHistogram(FILE *out);

  static const double sqrt2;
  static const double sqrt2Pi;

protected:
  
  bool _debug;
  bool _verbose;

  vector<double> _values;
  size_t _nVals;
  
  double _min;
  double _max;
  double _mean;
  double _median;
  double _mode;
  double _sdev;
  double _variance;
  double _skewness;
  double _kurtosis;

  double _histMin;
  double _histMax;
  double _histDelta;
  size_t _histNBins;
  double _histNSdev;

  vector<double> _histX;
  vector<double> _histCount;
  vector<double> _histDensity;
  vector<double> _histCumCount;
  vector<double> _histCumDensity;

  // mode and median from histogram

  double _histMedian;
  double _histMode;

  // fitting the pdf and cdf

  bool _pdfAvail;
  vector<double> _histPdf;
  vector<double> _histCdf;

  // mode and median from fitted pdf

  double _pdfMedian;
  double _pdfMode;

  // goodness of fit

  double _rmsePdf;
  double _gof;
  double _smk;
  double _smk95;

  virtual void _clearStats();
  virtual void _clearHist();

private:
  
};

#endif
