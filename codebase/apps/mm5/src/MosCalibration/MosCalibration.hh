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
//  mosCalibration top-level application class
//
//  $Id: MosCalibration.hh,v 1.7 2016/03/07 01:33:50 dixon Exp $
//
////////////////////////////////////////////////////////////
#ifndef _MOS_CALIBRATION_INC
#define _MOS_CALIBRATION_INC

#include <string>
#include <math.h>

#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

class MosCalibration
{
public:

   MosCalibration();
  ~MosCalibration(){};

   //
   // Initialization
   //
   int                 init( int argc, char **argv );
   const string&       getProgramName(){ return program.getFile(); }

   //
   // Messaging
   //
   MsgLog&             getMsgLog(){ return msgLog; }

   //
   // Execution
   //
   int                 run();

   //
   // Constants
   //
   static const int DAYS_TO_SEC;

   
private:

   //
   // Initialization
   //
   Path               program;

   void               usage();
   int                processArgs( int argc, char **argv );

   //
   // Parameter processing
   //
   char              *paramsPath;
   tdrp_override_t    tdrpOverride;
   Params             params;
   int                readParams( int argc, char **argv );
   int                processParams();

   //
   // Times
   //
   time_t             startTime;
   time_t             endTime;

   //
   // Messaging
   //
   MsgLog             msgLog;

   //
   // Data management and execution
   //
   DataMgr            dataMgr;
};

//
// Make one instance global
//
#ifdef _MOS_CALIB_MAIN_
          MosCalibration *mosCalibration;
#else
   extern MosCalibration *mosCalibration;
#endif

// 
// Macros for message logging 
// 
#define POSTMSG       mosCalibration->getMsgLog().postMsg
#define DEBUG_ENABLED mosCalibration->getMsgLog().isEnabled( DEBUG )
#define INFO_ENABLED  mosCalibration->getMsgLog().isEnabled( INFO )
   
//
// Macro for easy access to application name
//
#define PROGRAM_NAME mosCalibration->getProgramName().c_str()

//
// Defines for returns
//
#define SUCCESS 0
#define FAILURE -1

//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
