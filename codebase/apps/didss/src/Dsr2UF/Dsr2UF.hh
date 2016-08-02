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
#ifndef Dsr2UF_H
#define Dsr2UF_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class Dsr2UF {
  
public:

  // constructor

  Dsr2UF (int argc, char **argv);

  // destructor
  
  ~Dsr2UF();

  // run 

  int Run();

  // check constructor OK

  int isOK;

protected:
  
private:
  
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  char _tmpPath[MAX_PATH_LEN];
  FILE *_out;
  time_t _beamTime;
  time_t _startTime;
  time_t _endTime;

  int _scanType;
  int _scanMode;
  int _rayNumInVol;

  bool _snrCensorWarningPrinted;

  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg,
	       int &contents,
	       bool &end_of_vol);

  int _openOutputFile();
  int _closeOutputFile();

  void _censorOnSnr(DsRadarMsg &radarMsg);

};

#endif
