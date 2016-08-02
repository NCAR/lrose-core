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
/*
 *   Module: Dsr2Radar.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   10/5/01
 *
 *   Description: Class Dsr2Radar creates a RSL Radar struct from
 *                DsrRadarBeams. Dsr2Rsl object decodes DsRadarBeam mesages, 
 *                reformats the data into RSL radar structs, and has 
 *                methods for writing to fmq.
 */


#ifndef DSR2RSL_H
#define DSR2RSL_H

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <trmm_rsl/rsl.h>
#include "Dsr2Radar.hh"
#include "FourDD.hh"
using namespace std;

class Dsr2Rsl {
  
public:

  // constructor

  Dsr2Rsl (int argc, char **argv);

  // destructor
  
  ~Dsr2Rsl();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  Dsr2Radar *currRadarVol;
  Dsr2Radar *prevRadarVol;
  FourDD    *fourDD;

  // functions
  
  int _run();

  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg,
	       bool &end_of_vol,
	       int &contents);

  void _processVol();
  void _writeVol(DsRadarQueue &outputQueue);
  void _reset();
  
};

#endif






