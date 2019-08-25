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
// Beam.hh
//
// Beam object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <cstdio>
#include <radar/RadarComplex.hh>
#include <radar/InterestMap.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/KdpFilt.hh>
#include <radar/AtmosAtten.hh>
#include <radar/AparAltModeVel.hh>
#include <radar/AparMoments.hh>
#include <radar/AparGateData.hh>
#include <radar/AparMomFields.hh>
#include <radar/AparTsInfo.hh>
#include <radar/AparTsPulse.hh>
#include <radar/AparTsCalib.hh>
#include "Params.hh"
// #include "MomentsMgr.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // scan mode for determining PPI vs RHI operations
  
  typedef enum {
    SCAN_TYPE_UNKNOWN,
    SCAN_TYPE_PPI,
    SCAN_TYPE_RHI,
    SCAN_TYPE_VERT
  } scan_type_t;
  
  // Constructor
  
  Beam(const string &progName,
       const Params &params);
        
  // initialize before use
  
  void init(int nSamples,
            int nGates,
            int nGatesPrtLong,
            double elevation,
            double azimuth,
            scan_type_t scanType,
            bool isAlternating,
            bool isStagPrt,
            double prt,
            double prtLong,
            bool endOfSweepFlag,
            bool endOfVolFlag,
            const AtmosAtten &atmosAtten,
            const AparTsInfo &opsInfo,
            const vector<const AparTsPulse *> &pulses);

  // destructor
  
  ~Beam();

  // compute moments
  
  void computeMoments();

  // set methods

  void setTargetElevation(double el) { _targetEl = el; }
  void setCalib(const AparTsCalib &calib);
  void setStatusXml(const string &statusXml);

  void setSweepNum(int val) { _sweepNum = val; }
  void setVolNum(int val) { _volNum = val; }

  // get methods

  int getNSamples() const { return _nSamples; }

  double getElevation() const { return _el; }
  double getAzimuth() const { return _az; }
  double getTargetElevation() const { return _targetEl; }
  double getTargetAzimuth() const { return _targetAz; }
  double getPrt() const { return _prt; }
  bool getIsStagPrt() const { return _isStagPrt; }
  int getStagM() const { return _stagM; }
  int getStagN() const { return _stagN; }
  double getPrtShort() const { return _prtShort; }
  double getPrtLong() const { return _prtLong; }
  double getPulseWidth() const { return _pulseWidth; }
  double getNyquist() const { return _nyquist; }

  time_t getTimeSecs() const { return _timeSecs; }
  time_t getNanoSecs() const { return _nanoSecs; }
  double getDoubleTime() const { return _dtime; }
  
  int getScanMode() const;
  int getSweepNum() const { return _sweepNum; }
  int getVolNum() const { return _volNum; }
  bool getEndOfSweepFlag() const { return _endOfSweepFlag; }
  bool getEndOfVolFlag() const { return _endOfVolFlag; }

  bool getIsAlternating() const { return _isAlternating; }

  int getNGates() const { return _nGates; }
  int getNGatesOut() const { return _nGatesOut; }
  double getMaxRange() const;

  double getStartRange() const { return _startRangeKm; }
  double getGateSpacing() const { return _gateSpacingKm; }

  bool getIsDualPol() const { return _dualPol; }
  bool getIsSwitchingReceiver() const { return _switchingReceiver; }

  const AparTsInfo &getOpsInfo() const { return _opsInfo; }

  const AparMomFields* getFields() const { return _fields; }

  const AparTsCalib &getCalib() const { return _calib; }
  const string &getStatusXml() const { return _statusXml; }

  const apar_ts_platform_georef_t &getGeoref() const { return _georef; }
  bool getGeorefActive() const { return _georefActive; }

  bool hasMissingPulses() const { return _hasMissingPulses; }

  // manage threading

  static void setUpThreads(const Params &params);
  static void cleanUpThreads();

protected:
  
private:

  static const double _missingDbl;
  static int _nWarnings;

  string _progName;
  const Params &_params;

  // MomentsMgr _mmgr;

  // pulse vector

  vector<const AparTsPulse *> _pulses;
  bool _hasMissingPulses;

  // number of samples

  int _nSamples;
  int _nSamplesHalf;
  int _nSamplesAlloc;

  // number of gates, gate geometry
  
  int _nGates; // input data
  int _nGatesOut; // output
  int _nGatesOutAlloc; // output

  // time and location

  time_t _timeSecs;
  int _nanoSecs;
  double _dtime;

  double _el;
  double _az;
  double _targetEl;
  double _targetAz;

  int _scanMode;
  int _sweepNum;
  int _volNum;
  bool _endOfSweepFlag;
  bool _endOfVolFlag;
  
  scan_type_t _scanType;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // platform georeferencing - for moving platforms

  apar_ts_platform_georef_t _georef;
  bool _georefActive;

  // atmospheric attenuation

  const AtmosAtten *_atmosAtten;

  // modes

  bool _isAlternating;
  bool _dualPol;
  bool _switchingReceiver;

  // prt and pulse width

  double _prt;
  double _nyquist;
  double _pulseWidth;

  // staggered PRT mode

  bool _isStagPrt;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  int _nGatesStagPrt;
  double _nyquistPrtShort;
  double _nyquistPrtLong;
  int _stagM;
  int _stagN;

  // manager for computing moments

  AparTsInfo _opsInfo;

  // moments fields at each gate

  AparMomFields *_fields;

  // calibration

  AparTsCalib _calib;

  // status XML

  string _statusXml;

  // Moments computations
  
  AparMoments *_mom;

  // KDP

  bool _needKdp;
  KdpFilt _kdp;

  TaArray<double> _snrArray_;
  TaArray<double> _dbzArray_;
  TaArray<double> _zdrArray_;
  TaArray<double> _rhohvArray_;
  TaArray<double> _phidpArray_;

  double *_snrArray;
  double *_dbzArray;
  double *_zdrArray;
  double *_rhohvArray;
  double *_phidpArray;

  // fields for moments

  TaArray<AparMomFields> _momFields_;
  AparMomFields *_momFields;

  // alternating velocity

  AparAltModeVel _altVel;
  
  // gate data

  vector<AparGateData *> _gateData;
  
  // debug printing
  
  static pthread_mutex_t _debugPrintMutex;

 // private functions
  
  void _releasePulses();
  void _prepareForComputeMoments();
  int _getVolNum();
  int _getSweepNum();

  void _overrideOpsInfo();
  void _initMomentsObject();
  
  void _kdpInit();
  void _kdpCompute();

  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();
  void _loadGateIq(const fl32 **iqChan0, const fl32 **iqChan1);
  void _initStagPrt(int nGatesPrtShort, int nGatesPrtLong,
                    double prtShort, double prtLong);

  void _copyDataToOutputFields();
  int _checkCalib();

  void _applyMedianFilterToZDR(int nGates);
  void _applyMedianFilterToRHOHV(int nGates);
  void _cleanUpStagVel();
  
};

#endif

