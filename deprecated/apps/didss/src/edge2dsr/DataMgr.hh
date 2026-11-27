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
////////////////////////////////////////////////////////////////////////////
// Data managment class
///////////////////////////////////////////////////////////////////////////

#ifndef _EDGE_DATA_MGR_
#define _EDGE_DATA_MGR_

#include <cstdio>
#include <dataport/port_types.h>

#include "BeamWriter.hh"
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class MsgLog;
class EdgeTape;
class EdgeTcpip;
class EdgeUdp;
class EdgeMsg;
class DsFmq;
class DsRadarQueue;
class DsRadarMsg;
class DsRadarParams;
class DsRadarBeam;

class DataMgr {

public:

   enum inputType { UDP, TCPIP, FMQ, TAPE };

   DataMgr();
   ~DataMgr();
   
   int   init( Params& params, MsgLog* msgLog );
   int   processData();

   static const int    MAX_BUF_SIZE;
   
private:

   //
   // Input messages
   //
   inputType                 inputMode;
   EdgeTcpip                *edgeTcpip;
   EdgeUdp                  *edgeUdp;
   DsFmq                    *edgeFmq;
   EdgeTape                 *edgeTape;
   int                       bufferLen;
   char                     *buffer;
   EdgeMsg                  *edgeMsg;

   //
   // Intermediate information
   //
   bool                      paramsRead;


   //
   // Output message writer
   //
   BeamWriter                *_beamWriter;
  
   //
   // Archive messages
   //
   DsFmq                    *archiveQueue;
   FILE                     *archiveFp;

   //
   // Diagnostics
   //

};

#endif
