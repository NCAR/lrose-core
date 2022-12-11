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
// Stats.hh
//
// Statistical distribution - copied from libs/rapmath and modified
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#ifndef Stats_hh
#define Stats_hh

#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
using namespace std;

////////////////////////
// Base class
//
// Missing or unset values are set to NAN

class Stats {
  
public:
  
  // constructor
  
  Stats();
  
  // destructor
  
  virtual ~Stats();
  
  // initialize histogram range
  // the min val is the mid val of the first bin
  // the max val is the mid val of the last bin
  
  void init(double minVal, double maxVal, size_t nBins);

  // add a data value
  
  void addValue(double xx);

  // set the data values
  
  void setValues(const vector<double> &vals);

  // clear all

  void clear();

  // compute the stats

  void computeStats();

  // get methods

  size_t getNValues() const { return _nVals; }
  
  double getMin() const { return _min; }
  double getMax() const { return _max; }
  double getMean() const { return _mean; }
  double getSdev() const { return _sdev; }
  double getVariance() const { return _variance; }
  double getSkewness() const { return _skewness; }
  double getKurtosis() const { return _kurtosis; }
  double getMedian() const { return _median; }
  double getMode() const { return _mode; }
  
  size_t getHistNBins() const { return _histNBins; }
  double getHistMin() const { return _histMin; }
  double getHistMax() const { return _histMax; }
  double getHistDelta() const { return _histDelta; }

  const vector<double> &getHistX() const { return _histX; }
  const vector<double> &getHistCount() const { return _histCount; }


  // print histogram as text
  
  void printStats(FILE *out);
  void printHistogram(FILE *out);

protected:
  
  bool _initDone;

  double _nVals;
  double _sum;
  double _sumSq;
  
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
  
private:
  
};

#endif
