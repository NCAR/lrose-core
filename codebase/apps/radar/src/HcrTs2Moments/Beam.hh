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
// April 2005
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <radar/RadarMoments.hh>
#include <radar/RadarComplex.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/GateData.hh>
#include <radar/MomentsFields.hh>
#include <radar/InterestMap.hh>
#include <radar/NoiseLocator.hh>
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // scan mode for determining PPI vs RHI operations
  
  typedef enum {
    SCAN_TYPE_UNKNOWN,
    SCAN_TYPE_RHI,
    SCAN_TYPE_VERT,
    SCAN_TYPE_POINT
  } scan_type_t;
  
  // Constructor
  
  Beam(const string &progName,
       const Params &params);
        
  // initialize before use
  
  void init(int nSamples,
            int nGates,
            double pulseWidthUs,
            double prt,
            double el,
            double az,
            double elRate,
            scan_type_t scanType,
            iwrf_xmit_rcv_mode_t xmitRcvMode,
            int blockId,
            const string &blockName,
            const IwrfTsInfo &opsInfo,
            const vector<shared_ptr<IwrfTsPulse>> &pulses);

  // destructor
  
  ~Beam();

  // compute moments
  
  void computeMoments();

  // set methods

  void setTargetEl(double el) { _targetEl = el; }
  void setCalib(const IwrfCalib &calib);
  void setStatusXml(const string &statusXml);
  void appendStatusXml(const string &extraXml);

  // get methods

  int getNSamples() const { return _nSamples; }
  int getBlockId() const { return _blockId; }
  string getBlockName() const { return _blockName; }

  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getTargetEl() const { return _targetEl; }
  double getTargetAz() const { return _targetAz; }
  double getPrt() const { return _prt; }
  double getPulseWidthUs() const { return _pulseWidthUs; }
  double getNyquist() const { return _nyquist; }
  double getUnambigRangeKm() const;

  time_t getTimeSecs() const { return _timeSecs; }
  long int getNanoSecs() const { return _nanoSecs; }
  double getDoubleTime() const { return _dtime; }
  
  Beam::scan_type_t getScanType() const { return _scanType; }

  int getNGates() const { return _nGates; }
  int getNGatesOut() const { return _nGatesOut; }
  double getMaxRangeKm() const;

  double getStartRangeKm() const { return _startRangeKm; }
  double getGateSpacingKm() const { return _gateSpacingKm; }

  iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }

  const IwrfTsInfo &getOpsInfo() const { return _opsInfo; }

  const MomentsFields* getFields() const { return _fields; }

  const IwrfCalib &getCalib() const { return _calib; }
  const string &getStatusXml() const { return _statusXml; }

  const iwrf_platform_georef_t &getGeoref() const { return _georef; }
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

  // block identification

  int _blockId;
  string _blockName;
  
  // pulse vector

  vector<shared_ptr<IwrfTsPulse>> _pulses;
  bool _hasMissingPulses;
  static pthread_mutex_t _pulseUnpackMutex;

  // number of samples
  
  int _nSamples; // nsamples making adjustment for window
  int _nSamplesHalf;
  int _nSamplesAlloc;

  // number of gates, gate geometry
  
  int _nGates; // input data
  int _nGatesOut; // output
  int _nGatesOutAlloc; // output
  
  // time and location

  time_t _timeSecs;
  long int _nanoSecs;
  double _dtime;

  double _meanPointingAngle;
  double _el;
  double _az;
  double _targetEl;
  double _targetAz;
  double _elRate;

  int _scanMode;
  scan_type_t _scanType;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // platform georeferencing - for moving platforms

  iwrf_platform_georef_t _georef;
  bool _georefActive;

  // modes

  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  
  // prt and pulse width

  double _prt;
  double _nyquist;
  double _pulseWidthUs;

  // manager for computing moments
  
  IwrfTsInfo _opsInfo;
  double _wavelengthM;

  // moments fields at each gate

  MomentsFields *_fields;
  
  // calibration
  
  IwrfCalib _calib;

  // status XML

  string _statusXml;

  // Moments computations
  
  RadarMoments *_mom;
  
  // window
  
  double *_window;
  double *_windowHalf;
  double *_windowVonHann;
  
  // R values, at various lags, for the windows
  // used to correct R values used in width computations

  double _windowR1;
  double _windowR2;
  double _windowR3;
  double _windowHalfR1;
  double _windowHalfR2;
  double _windowHalfR3;

  // noise

  NoiseLocator *_noise;

  // fields for moments

  TaArray<MomentsFields> _momFields_;
  MomentsFields *_momFields;

  // censoring
  
  TaArray<bool> _censorFlags_;
  bool *_censorFlags;

  // gate data

  vector<GateData *> _gateData;
  
  // FFTs

  static pthread_mutex_t _fftMutex;
  RadarFft *_fft;
  RadarFft *_fftHalf;

  // debug printing
  
  static pthread_mutex_t _debugPrintMutex;

 // private functions
  
  void _freeWindows();
  void _prepareForComputeMoments();
  int _getVolNum();
  int _getSweepNum();
  void _computeMoments();

  void _computeMomDpHOnly();
  void _computeMomDpVOnly();

  void _computeWindowRValues();
  void _overrideOpsInfo();
  void _computeWindows();
  void _initMomentsObject(RadarMoments *mom);

  void _computeVelocityCorrectedForVertMotion();
  void _copyVelToCorrectedVel();
  void _computeVelocityCorrectedForMotion();
  double _correctForNyquist(double vel);
  void _computeWidthCorrectedForMotion();

  int _noiseInit();
  int _convertInterestParamsToVector(const string &label,
                                     const Params::interest_map_point_t *map,
                                     int nPoints,
                                     vector<InterestMap::ImPoint> &pts);


  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();
  void _loadGateIq(const fl32 **iqChan0, const fl32 **iqChan1);
  
  void _copyDataToOutputFields();
  int _checkCalib();

  int _correctCalibGainsForTemp();
  int _correctHcrVRxGainForTemp();

  double _getTempFromTagList(const string &tagList) const;
  double _getDeltaGainFromXml(const string &xml,
                              const string &tagList) const;
  
  void _printSelectedMoments();

  void _setNoiseFields();
  void _censorByNoiseFlag();
  void _censorBySnrNcp();
  void _censorFields(MomentsFields &mfield);
  void _fillInCensoring(size_t nGates);
    
  double _conditionAz(double az);
  double _conditionEl(double el);
  
  void _correctAltitudeForEgm();
  void _setGeorefForFixedMode();

};

#endif

