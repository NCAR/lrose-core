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
// ProSensing2Dsr.hh
//
// ProSensing2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// ProSensing2Dsr reads radar data from a ProSensing radar server
// and writes it to an FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#ifndef ProSensing2Dsr_HH
#define ProSensing2Dsr_HH

#include <string>
#include <toolsa/MemBuf.hh>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarCalib.hh>
#include <radar/AtmosAtten.hh>
#include <radar/MomentsFields.hh>
#include "Args.hh"
#include "Params.hh"
#include "XpolComms.hh"
using namespace std;

////////////////////////
// This class

class ProSensing2Dsr {
  
public:

  // constructor

  ProSensing2Dsr(int argc, char **argv);

  // destructor
  
  ~ProSensing2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsRadarQueue _rQueue;

  XpolComms _xpol;

  time_t _rayTimeSecs;
  int _rayNanoSecs;
  double _elDeg;
  double _azDeg;
  int _rayCount;
  int _volNum;
  int _tiltNum;
  int _scanType;

  int _nGates;
  int _prevNGates;
  double _startRange;
  double _gateSpacing;
  double _prevStartRange;
  double _prevGateSpacing;

  // calibration and attenuation

  DsRadarCalib _calib;
  AtmosAtten _atten;
  bool _attenReady;

  double _calNoiseDbmHc;
  double _calNoisePowerHc;
  double _calNoiseDbmVc;
  double _calNoisePowerVc;

  double _estNoisePowerHc;
  double _estNoisePowerVc;

  double _minDetectableSnr;

  double _rxGainDbHc;
  double _rxGainDbVc;
  double _dbz0;
  double _zdrCorrectionDb;
  double _wavelengthM;

  // radar parameters
  
  int _samplesPerBeam;
  double _prf;

  // range correction

  int _rCorrNGates;
  double _rCorrStartRange;
  double _rCorrGateSpacing;
  double *_range;
  double *_rangeCorrection;

  // number of active fields

  int _nFields;

  // velocity

  double _prt;
  double _nyquist;
  double _velSign;

  // staggered PRT velocity

  double _prtShort, _prtLong;
  int _staggeredM, _staggeredN;
  double _nyquistPrtShort, _nyquistPrtLong;
  int _LL;
  int _PP_[32];
  int *_PP;

  // phidp

  double _phidpPhaseLimitSim;
  double _systemPhidp;
  RadarComplex_t _phidpOffset;

  // moments comps
  
  vector<MomentsFields> _moments;

  // functions

  int _processRay();
  int _writeRadarAndFieldParams();
  int _writeCalib();
  int _writeRay();

  void _computeRangeCorrTable();

  void _computeMomentsPp();
  void _computeMomentsPpSumPowers();
  void _computeMomentsPp(int gateNum,
                         double range,
                         double lag0_hc,
                         double lag0_vc,
                         const RadarComplex_t &lag1_hc,
                         const RadarComplex_t &lag1_vc,
                         const RadarComplex_t &Rvvhh0,
                         MomentsFields &mom);
  
  void _computeMomentsStagPp();
  void _computeMomentsStagPp(int gateNum,
                             double range,
                             double lag0_hc_short_to_long,
                             double lag0_hc_long_to_short,
                             double lag0_vc_short_to_long,
                             double lag0_vc_long_to_short,
                             const RadarComplex_t &lag1_hc_short_to_long,
                             const RadarComplex_t &lag1_hc_long_to_short,
                             const RadarComplex_t &lag1_vc_short_to_long,
                             const RadarComplex_t &lag1_vc_long_to_short,
                             const RadarComplex_t &Rvvhh0,
                             MomentsFields &mom);

  void _computeMomentsStagPpSumPowers();

  double _computeR0R1Width(double r0, double r1, double nyquist);

  double _computeWidth(double rA,
                       double rB,
                       int lagA,
                       int lagB,
                       double nyquist);

  void _initStaggered();

};

#endif
