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
//
// Inits FMQ in constructor, sends data in method.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef LASSEN_DSR
#define LASSEN_DSR

#include <dataport/port_types.h>
#include <toolsa/umisc.h>

#include <vector>

#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>

#include "Params.hh"

using namespace std;

class lassenNetcdf2Dsr {
  
public:

  // Constructor. Inits output FMQ.
  lassenNetcdf2Dsr ( Params *TDRP_params );

  // Process volumes.
  void processFilePair( char *zFile, char *uFile, 
			time_t dataTime, int tiltNum );

  // Send an end of volume to the FMQ.
  void sendEOV();

  // Destructor.
  ~lassenNetcdf2Dsr ();

protected:
  
private:


  // Small local routine to check netCDF reads.
  int _checkStatus(int status, char *exitStr);

  int _volNum;
  float _lastTiltEl;

  bool _firstTilt;
  time_t _saveTime;

  Params *_params;


  DsRadarQueue  _radarQueue;
  MsgLog        _msgLog;
  DsRadarMsg    _radarMsg;

  const static int _maxFields=2; // Implied for this dataset.
  DsFieldParams *_fieldParams[_maxFields];

  const static fl32 MISSING_FLOAT = -9999.0;
  const static fl32 SCALE = 1.0;
  const static fl32 BIAS = 0.0;  
  const static int DATA_BYTE_WIDTH = 4;

};

#endif





