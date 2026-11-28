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
// Windtracer2Dsr.hh
//
// Windtracer2Dsr object
//
//
///////////////////////////////////////////////////////////////


#ifndef Windtracer2Dsr_H
#define Windtracer2Dsr_H

#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>
#include <vector>

using namespace std;

class Windtracer2Dsr {
  
public:
  
  // constructor. Sets up DsMdvx object.
  Windtracer2Dsr (Params *TDRP_params);

  // 
  void  Windtracer2DsrFile( char *filename); 
  
  // destructor.
  ~Windtracer2Dsr();

  
protected:
  
private:

  Params *      _params;

  int _year, _month, _day;
  int _hour, _min, _sec, _msec;
  double _beamTime;
  double _beamDayTime;
  double _el, _az;

  double _firstRange, _deltaRange;

  DsRadarQueue  _radarQueue; 
  MsgLog        _msgLog;
  int           _volumeNum;
  int           _numExpectedFields;
  int           _numGates;
  DsRadarMsg    _radarMsg; 
  int           _numFieldsSoFar;
  int           _beamsSentSoFar;
  long int      _fileTimeOffset;
  bool          _firstTime;
  bool          _firstAz;
  double        _lastAz;
  double        _delAz;
  double        _signedDelAz;
  double        _lastSignedDelAz;
  time_t        _fileTime;

  double        _volumeStartTime;
  bool          _hasVolumeStartedYet;

  bool          _onLastScan;
  bool          _reachedFirstElevation;
  bool          _reachedFirstAzimuth;
  int           _numOnLastScan;
  bool          _timeToCloseFile;
  bool          _azReversal;

  const static int _maxFields=20;
  DsFieldParams *_fieldParams[_maxFields];

  float *_beamData;

  const static float MISSING_FLOAT;
  const static float SCALE;
  const static float BIAS;  
  const static int DATA_BYTE_WIDTH = 4;
  const static int SECONDS_PER_DAY = 86400;

  int _readShort(FILE **fp);
  int _readLong(FILE **fp);
  void _printID( int ID );
  void _byteSwap4(void *p);
  void _readKeyword(FILE *fp, char *key, double *val, int tryRead);
 
  void _processSpecial(FILE *fp);
  void _processHeader(FILE *fp);
  void _processData(FILE *fp);
  void _processText(FILE *fp, int len);
  void _shearTest(float *_beamData);
  void _processExistingText();
  int _readLocationFile(time_t DataTime, double *lat, 
			double *lon, double *alt);


};

#endif
