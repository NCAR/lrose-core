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
// Dsr2Mdv.h
//
// Dsr2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef Dsr2Mdv_H
#define Dsr2Mdv_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>

#include "Args.hh"
#include "Params.hh"
#include "Resample.hh"
using namespace std;


class Dsr2Mdv {
  
public:

  // constructor

  Dsr2Mdv (int argc, char **argv);

  // destructor
  
  ~Dsr2Mdv();

  // run 

  int Run();

  // data members

  int OK;
  int Done;
  char *_progName;
  Args *_args;
  Params *_Params;
  Dsr2Mdv_tdrp_struct *_params;

protected:
  
private:

  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg,
	       Lookup *lookup,
	       int &has_beam_data,
	       int &end_of_vol);

  int _scanType;

};

#endif
