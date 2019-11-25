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
// TimeSample.hh
//
// Representing data at one time
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2018
//
/////////////////////////////////////////////////////////////

#ifndef TIME_SAMPLE_HH
#define TIME_SAMPLE_HH

using namespace std;
#include "Params.hh"
#include <ctime>

class TimeSample {
  
public:

  // constructor

  TimeSample(const Params &params);

  // Destructor

  ~TimeSample();

  // set methods

  void clear();
  void setTime(time_t timeVal) { _time = timeVal; }

  void addLnaTempObs(double temp);
  void addRxTempObs(double temp);

  void setLnaSmoothedN(double val) { _lnaSmoothedN = val; }
  void setRxSmoothedN(double val) { _rxSmoothedN = val; }

  void setLnaTempSmoothed(double val) { _lnaTempSmoothed = val; }
  void setRxTempSmoothed(double val) { _rxTempSmoothed = val; }

  // compute means

  void computeMeanObs();

  // compute delta gain
  // returns 0 on success, -1 on error

  int computeDeltaGain(double lnaRefTempC,
                       double lnaGainChangePerC,
                       double rxRefTempC,
                       double rxGainChangePerC);

  // get methods

  time_t getTime() const { return _time; }

  double getLnaTempSum() const { return _lnaTempSum; }
  double getRxTempSum() const { return _rxTempSum; }

  double getLnaTempN() const { return _lnaTempN; }
  double getRxTempN() const { return _rxTempN; }

  double getLnaTempMean() const { return _lnaTempMean; }
  double getRxTempMean() const { return _rxTempMean; }

  double getLnaSmoothedN() const { return _lnaSmoothedN; }
  double getRxSmoothedN() const { return _rxSmoothedN; }

  double getLnaTempSmoothed() const { return _lnaTempSmoothed; }
  double getRxTempSmoothed() const { return _rxTempSmoothed; }
  
  double getLnaDeltaGain() const { return _lnaDeltaGain; }
  double getRxDeltaGain() const { return _rxDeltaGain; }
  double getSumDeltaGain() const { return _sumDeltaGain; }
  
protected:
  
private:

  const Params &_params;

  time_t _time;
  
  double _lnaTempSum;
  double _lnaTempN;
  double _lnaTempMean;
  double _lnaTempSmoothed;
  double _lnaSmoothedN;
  
  double _rxTempSum;
  double _rxTempN;
  double _rxTempMean;
  double _rxTempSmoothed;
  double _rxSmoothedN;

  double _lnaDeltaGain;
  double _rxDeltaGain;
  double _sumDeltaGain;
  
};

#endif



