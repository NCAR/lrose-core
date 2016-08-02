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
// superResNexradII2Dsr.hh
//
// Inits FMQ in constructor. ProcVols() method processes volumes.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef SUPER_RES_NEX_II
#define SUPER_RES_NEX_II

#include <dataport/port_types.h>
#include <toolsa/umisc.h>

#include <vector>

#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>

#include "Params.hh"

using namespace std;

class superResNexradII2Dsr {
  
public:

  // Constructor. Inits output FMQ.
  superResNexradII2Dsr ( Params *TDRP_params );

  // Process volumes.
  int procVols( char *Filename );

  // Destructor.
  ~superResNexradII2Dsr ();

protected:
  
private:

  Params *_params;

  int _vcpNum;
  int _volNum;

  int _numFields;

  int _nGates;
  int _nAz;

  int _lastMessageType;

  time_t _lastBeamTimeSent;

  DsRadarQueue  _radarQueue; 
  MsgLog        _msgLog;
  DsRadarMsg    _radarMsg;

  bool _sendBeam;
  bool _sendElevStart;
  bool _sendElevEnd;
  bool _sendVolStart;
  bool _sendVolEnd;

  date_time_t _filenameTime;

  fl32 _az;
  fl32 _el;

  int _elNum;
  int _outputTiltNum;

  typedef struct {
    double min;
    double max;
    double targetElev;
  } elevRange_t;

  typedef struct {
    double beamElev;
    time_t beamTime;
  } beamMetaData_t;

  vector <elevRange_t> _elevRanges;

  typedef struct {
    string fieldName;
    int elevNum;
  } elevSelect_t;

  vector <elevSelect_t> _elevSelections;

  vector <int> _writeAfterTheseElevs;

  void _bSwap(void *i);
  void _bSwapLong(void *i);
  void _message31(unsigned char *buf, int bufSize);
  void _volumeHeader(unsigned char *buf);
  double _lookupElev(double el );
  bool _isActive( int eleNum, string fieldName );
  void _resetInternalFlags();

  void _getTime(int jDate, int mSecPastMidnight, date_time_t *ralTime);

  void _initTiltToMissing();
  void _readVCPinfoFile();

  typedef struct {
    ui16 size;
    ui08 channel;
    ui08 msgType;
    ui16 msgSeqNum;
    ui16 jDate;
    ui32 numSecMidnight;
    ui16 totalMsgSegs;
    ui16 msgSegNum;
  } header_t;

  const static int _maxFields=6; // Implied for this dataset.
  DsFieldParams *_fieldParams[_maxFields];

  fl32 *_beamData;
  beamMetaData_t *_beamMetaData;

  const static fl32 MISSING_FLOAT;
  const static fl32 SCALE;
  const static fl32 BIAS;
  const static int DATA_BYTE_WIDTH = 4;
  const static int SECONDS_PER_DAY = 86400;

  double _lat, _lon, _altKm;

  double _nyquistVel, _currentNyquistVel;

};

#endif





