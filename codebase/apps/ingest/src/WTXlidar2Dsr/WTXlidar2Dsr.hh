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
// WTXlidar2Dsr.hh
//
// WTXlidar2Dsr object
//
//
///////////////////////////////////////////////////////////////


#ifndef WTXlidar2Dsr_H
#define WTXlidar2Dsr_H

#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>
#include <vector>

using namespace std;

class WTXlidar2Dsr {
  
public:
  
  // constructor. Sets up DsMdvx object.
  WTXlidar2Dsr (Params *TDRP_params);

  // 
  void  WTXlidar2DsrFile( char *filename); 
  
  // destructor.
  ~WTXlidar2Dsr();

  
protected:
  
private:

  Params *      _params;

  time_t _dataTime, _volumeStartTime;

  double _firstRange, _deltaRange;

  DsRadarQueue  _radarQueue; 
  MsgLog        _msgLog;
  int           _numGates;
  DsRadarMsg    _radarMsg; 
  double        _az;
  double        _el;
  int _volumeNum;

  int _timingID;


  const static int _numFields=6;
  DsFieldParams *_fieldParams[_numFields];

  fl32 *_beamData;

  const static float MISSING_FLOAT = -999.0;
  const static float SCALE = 1.0;
  const static float BIAS = 0.0;  
  const static int DATA_BYTE_WIDTH = 4;


  double _parseConfigElement(char *config, char *keyStr, bool &ok);
  void _clearData();
  int _getFieldIndex(int fieldID);

  typedef struct __attribute__ ((__packed__)) {
    ui16 nId;
    ui16 nVersion;
    ui32 nBlockLength;
  } BlockDescriptor_t;


  typedef struct __attribute__ ((__packed__)) {
    ui32 nRecordLength;
    ui16 nYear;
    ui08 nMonth;
    ui08 nDayOfMonth;
    ui16 nHour;
    ui08 nMinute;
    ui08 nSecond;
    ui32 nNanosecond;
  } RecordHeader_t;

 typedef struct __attribute__ ((__packed__)) {
   fl32 fScanAzimuth_deg;
   fl32 fScanElevation_deg;
   fl32 fAzimuthRate_dps;
   fl32 fElevationRate_dps;
   fl32 fTargetAzimuth_deg;
   fl32 fTargetElevation_deg;
   ui32 nScanEnabled;
   ui32 nCurrentIndex;
   ui32 nAcqScanState;
   ui32 nDriverScanState;
   ui32 nAcqDwellState;
   ui32 nScanPatternType;
   ui32 nValidPos;
   ui32 nSSDoneState;
   ui32 nErrorFlags;
 } ScanInfo_t;

 typedef struct __attribute__ ((__packed__)) {
   fl32 fAzimuthMin_deg;
   fl32 fAzimuthMean_deg;
   fl32 fAzimuthMax_deg;
   fl32 fElevationMin_deg;
   fl32 fElevationMean_deg;
   fl32 fElevationMax_deg;
   fl32 fMonitorCount;
   fl32 fMonitorFrequency_hz;
   fl32 fMonitorTime;
   fl32 fMonitorPeak;
   fl32 fOverLevel;
   fl32 fUnderLevel;
 } ProductPulseInfo_t;
  


};

#endif
