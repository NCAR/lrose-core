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


# ifndef    PROCESS_IRIS_VOL_H
# define    PROCESS_IRIS_VOL_H

#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>
#include <toolsa/umisc.h>
#include <vector>

#include "Params.hh"
using namespace std;


class processIrisVol {
  
public:

  //
  // Constructor. Makes copies of params. Opens
  // output FMQ.
  //

  processIrisVol(Params *P);

  //
  // Init a new volume. Reads the base file, gets beam
  // spacing and some other things. Returns 0 on
  // success, else -1.
  //
  int initVol(char *baseName);

  //
  // Main gig. Process a tilt. Tilt numbers start at 0.
  // returns 0 if everything went OK, else -1.
  //
  int processTilt(char *baseName);

  //
  // Destructor. Does little, but avoids default destructor.
  //
  ~ processIrisVol();


protected:
private:

  Params *_params;
  int _numFieldsOut;

  DsRadarQueue  _radarQueue; 
  MsgLog        _msgLog;
  DsRadarMsg    _radarMsg;

  const static int _maxFields=20;
  DsFieldParams *_fieldParams[_maxFields];
   
  const static float SCALE;
  const static float BIAS;  
  const static int DATA_BYTE_WIDTH = 4;

  fl32 *_beamData;

  double _rangeFirstBinKm;
  double _binSpacingKm;
  int _numBins;

  double _origRangeFirstBinKm;
  double _origBinSpacingKm;

  double _pulseWidthMicroSec;
  double _prfHz;

  double _vertWidth;
  double _horizWidth;
  
  double _waveLen;

  double _lat;
  double _lon;
  double _alt;
  int _multiPrfFlag;
  double _nyquistVel;

  date_time_t _dataTime;
  int _tiltNum;
  int _volNum;

  int _beamsSent;

};


# endif
