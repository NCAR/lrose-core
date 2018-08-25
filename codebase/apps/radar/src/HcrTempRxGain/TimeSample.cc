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
//////////////////////////////////////////////////////////
// TimeSample.cc
//
// Representing data at one time
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2018
//
/////////////////////////////////////////////////////////////

#include "TimeSample.hh"
#include <toolsa/DateTime.hh>
#include <cmath>

//////////////////////////////////////////////////
// Constructor

TimeSample::TimeSample(const Params &params) :
        _params(params)
{
  
  clear();

}

////////////////////////////////////////////////////
// Destructor

TimeSample::~TimeSample()

{

}

//////////////////////////////////////////////////
// Clear

void TimeSample::clear()
{

  _time = DateTime::NEVER;
  
  _lnaTempSum = 0.0;
  _lnaTempN = 0.0;
  _lnaTempMean = NAN;
  _lnaSmoothedN = 0;
  _lnaTempSmoothed = NAN;
  
  _rxTempSum = 0.0;
  _rxTempN = 0.0;
  _rxTempMean = NAN;
  _rxSmoothedN = 0;
  _rxTempSmoothed = NAN;

  _lnaDeltaGain = NAN;
  _rxDeltaGain = NAN;
  _sumDeltaGain = NAN;

}

/////////////////////////////////////////////////////
// add observations

void TimeSample::addLnaTempObs(double temp)
{
  _lnaTempSum += temp;
  _lnaTempN++;
}

void TimeSample::addRxTempObs(double temp)
{
  _rxTempSum += temp;
  _rxTempN++;
}

/////////////////////////////////////////////////////
// compute the mean of the observations

void TimeSample::computeMeanObs()

{

  if (_lnaTempN > 0) {
    _lnaTempMean = _lnaTempSum / _lnaTempN;
  }

  if (_rxTempN > 0) {
    _rxTempMean = _rxTempSum / _rxTempN;
  }

}

/////////////////////////////////////////////////////
// compute the delta gain
// assumes smoothed temperatures have been set

void TimeSample::computeDeltaGain(double lnaRefTempC,
                                  double lnaGainChangePerC,
                                  double rxRefTempC,
                                  double rxGainChangePerC)

{

  if (!std::isnan(_lnaTempSmoothed)) {
    double deltaTemp =
      _lnaTempSmoothed - lnaRefTempC;
    double deltaGain = deltaTemp * lnaGainChangePerC;
    _lnaDeltaGain = deltaGain;
  }

  if (!std::isnan(_rxTempSmoothed)) {
    double deltaTemp =
      _rxTempSmoothed - rxRefTempC;
    double deltaGain = deltaTemp * rxGainChangePerC;
    _rxDeltaGain = deltaGain;
  }

  if (!std::isnan(_lnaDeltaGain) && !std::isnan(_rxDeltaGain)) {
    _sumDeltaGain = _lnaDeltaGain + _rxDeltaGain;
  } else if (!std::isnan(_lnaDeltaGain)) {
    _sumDeltaGain = _lnaDeltaGain;
  } else if (!std::isnan(_rxDeltaGain)) {
    _sumDeltaGain = _rxDeltaGain;
  }

}

