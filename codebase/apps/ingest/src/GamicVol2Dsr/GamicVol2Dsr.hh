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
// GamicVol2Dsr.hh
//
// GamicVol2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2009
//
///////////////////////////////////////////////////////////////
// GamicVol2Dsr reads GAMIC volume files and reformats
// the contents into a DsRadar FMQ.
///////////////////////////////////////////////////////////////

#ifndef GamicVol2Dsr_hh
#define GamicVol2Dsr_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Fmq/DsRadarQueue.hh>
#include "Args.hh"
#include "Params.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class GamicVol2Dsr {
  
public:

  // constructor

  GamicVol2Dsr (int argc, char **argv);

  // destructor
  
  ~GamicVol2Dsr();

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
  DsInputPath *_input;
  DsRadarQueue _rQueue;

  bool _firstBeamInFile;

  static const double _angleConversion;

  double _latitude;
  double _longitude;
  double _altitude;

  time_t _fileTime;
  int _fileNanosecs;

  time_t _latestBeamTime;
  int _latestBeamNanosecs;

  int _volType; // 1 == PPI, 2 == RHI
  int _tiltType; // 1 == PPI, 2 == RHI
  bool _isRhi;

  int _volNum;

  int _nTilts;
  int _nSamples;
  int _nAveragingInRange;

  int _nGatesOut;
  int _nGatesOutPrev;

  int _unfoldingFlag;
  int _longPulseFlag;
  int _clutterFlag;
  int _filterNum;

  int _angleSyncFlag;
  double _deltaAngle;
  
  double _beamWidthHoriz;
  double _beamWidthVert;
  double _pulseWidthUs;
  
  double _maxDbz;
  double _minDbz;

  double _prf;
  double _nyquist;
  double _wavelengthMeters;

  time_t _tiltTime;
  int _tiltNanosecs;
  
  double _targetAngle;
  double _startAngle;
  double _stopAngle;

  double _maxRange;
  double _startRange;
  double _gateSpacingMeters;
  double _gateSpacingKm;
  double _gateSpacingKmOut;

  string _scanName;
  string _radarName;
  string _hostName;

  int _nBeams;
  vector<Beam *> _beams;

  // functions

  int _freeBeams();
  int _processFile(const char *input_path);

  int _readHeader(FILE *in);
  int _readTilt(FILE *in, int tiltNum);

  int _readChar(FILE *in, const string &label, char &val);
  int _readUi16(FILE *in, const string &label, ui16 &val);
  int _readSi32(FILE *in, const string &label, si32 &val);
  int _readFl32(FILE *in, const string &label, fl32 &val);
  int _readFl64(FILE *in, const string &label, fl64 &val);
  int _readBuf(FILE *in, const string &label, char *buf, int len);
  int _skipBytes(FILE *in, int len);

  string _getStrFromBuf(char *buf, int len);

  int _computeTime(int jday, int msecsOfDay,
		   time_t &utime, int &nanosecs);

  int _writeTilt(int tiltNum);
  int _writeParams();

  int _writeBeam(int tiltNum, Beam &beam);
  void _loadRadarParams(DsRadarParams &rParams);
  void _loadOutputData(const char *inBuf, Beam *beam);
  void _remapOutputData(const char *inBuf, Beam *beam);

};

#endif

