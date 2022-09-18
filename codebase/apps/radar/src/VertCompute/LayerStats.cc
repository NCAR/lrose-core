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
// LayerStats.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2006
//
///////////////////////////////////////////////////////////////

#include "LayerStats.hh"
#include <toolsa/DateTime.hh>
#include <iomanip>
#include <cmath>
using namespace std;

// Constructor

LayerStats::LayerStats(const Params &params, double min_ht, double max_ht) :
        _params(params),
        _minHt(min_ht),
        _maxHt(max_ht)
  
{
  _meanHt = (_minHt + _maxHt) / 2.0;

  _globalNValid = 0;
  _globalSum.zeroOut();
  _globalSumSq.zeroOut();

}

// destructor

LayerStats::~LayerStats()

{

}

// clear data

void LayerStats::clearData()

{
  _nValid = 0;
  _momentData.clear();
  _dist.clearValues();
}

// add a zdr value

void LayerStats::addData(const MomentData &data)

{

  _momentData.push_back(data);

}
  
// print

void LayerStats::print(ostream &out)

{

  out << "============ LayerStats ==============" << endl;
  out << "  minHt: " << _minHt << endl;
  out << "  maxHt: " << _maxHt << endl;
  out << "  nData: " << _momentData.size() << endl;
  out << "  nValid: " << _nValid << endl;
  out << endl;

}

// compute stats

void LayerStats::computeStats()

{

  MomentData example;

  // mark data as invalid if it does not meet criteria

  for (int ii = 0; ii < (int) _momentData.size(); ii++) {
    double ldr = (_momentData[ii].ldrh + _momentData[ii].ldrv) / 2.0;
    if (_momentData[ii].snr < _params.min_snr ||
        _momentData[ii].snr > _params.max_snr ||
        _momentData[ii].vel < _params.min_vel ||
        _momentData[ii].vel > _params.max_vel ||
        _momentData[ii].rhohv < _params.min_rhohv ||
        ldr > _params.max_ldr) {
      _momentData[ii].valid = false;
    }
  }

  // compute mean and sdev of zdr data
  
  _computeZdrmMeanSdev(_meanZdr, _sdevZdr);
  
  // set invalid if zdr is outlier

  double minZdrm = _meanZdr - _sdevZdr * _params.zdr_n_sdev;
  double maxZdrm = _meanZdr + _sdevZdr * _params.zdr_n_sdev;
  for (int ii = 0; ii < (int) _momentData.size(); ii++) {
    if (_momentData[ii].zdrm < minZdrm ||
        _momentData[ii].zdrm > maxZdrm) {
      _momentData[ii].valid = false;
    }
  }

  // zero out sums

  _sum.zeroOut();
  _sumSq.zeroOut();

  // accumulate

  _nValid = 0;
  for (int ii = 0; ii < (int) _momentData.size(); ii++) {
    if (_momentData[ii].valid) {
      const MomentData &mdata = _momentData[ii];
      _sum.add(mdata);
      _sumSq.addSquared(mdata);
      _nValid++;
      _dist.addValue(mdata.zdrm);
      _globalDist.addValue(mdata.zdrm);
    }
  }

  _globalNValid += _nValid;
  _globalSum.add(_sum);
  _globalSumSq.add(_sumSq);

  // compute mean and sdev

  MomentData::computeMeanSdev(_nValid, _sum, _sumSq, _mean, _sdev);

  _dist.computeBasicStats();
  _dist.setHistRangeFromSdev(3.0);
  _dist.computeHistogram();
  _dist.performFit();
  _dist.computeGof();
  if (_nValid > 0 && _params.debug >= Params::DEBUG_VERBOSE) {
    print(cerr);
    _dist.printHistogram(stderr);
  }
  
}

// compute global stats

void LayerStats::computeGlobalStats()

{
  MomentData::computeMeanSdev(_globalNValid,
                              _globalSum, _globalSumSq,
                              _globalMean, _globalSdev);
  _globalDist.computeBasicStats();
  _globalDist.setHistRangeFromSdev(3.0);
  _globalDist.computeHistogram();
  _globalDist.performFit();
  _globalDist.computeGof();
  if (_globalNValid > 0 && _params.debug >= Params::DEBUG_VERBOSE) {
    print(cerr);
    _globalDist.printHistogram(stderr);
  }

}

// compute mean and sdev of zdrm
  
void LayerStats::_computeZdrmMeanSdev(double &mean, double &sdev)
  
{

  mean = MomentData::missingVal;
  sdev = MomentData::missingVal;
  
  double sum = 0;
  double sumSq = 0;
  double dn = 0;
  
  for (int ii = 0; ii < (int) _momentData.size(); ii++) {
    const MomentData &data = _momentData[ii];
    if (data.valid) {
      double val = data.zdrm;
      sum += val;
      sumSq += val * val;
      dn++;
    }
  } // ii

  if (dn > 0) {
    mean = sum / dn;
  }
  
  if (dn > 2) {
    double var = (sumSq - (sum * sum) / dn) / (dn - 1.0);
    if (var >= 0.0) {
      sdev = sqrt(var);
    } else {
      sdev = 0.0;
    }
  }

}
  
