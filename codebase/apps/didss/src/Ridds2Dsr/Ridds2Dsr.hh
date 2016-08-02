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
//////////////////////////////////////////////////////////////
// Ridds2Dsr top level application class
//
// $Id: Ridds2Dsr.hh,v 1.3 2016/03/06 23:53:41 dixon Exp $
//
/////////////////////////////////////////////////////////////
#ifndef _RIDDS_2_MOM_
#define _RIDDS_2_MOM_

#include <string>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

class Ridds2Dsr {

  public:

   Ridds2Dsr();
   ~Ridds2Dsr();
   
   //
   // Initialization
   //
   int init( int argc, char** argv );
   inline const string& getProgramName(){ return program.getFile(); }

   //
   // Messaging
   //
   inline MsgLog& getMsgLog() { return msgLog; }
   
   //
   // Execution
   //
   int run();

  private:

   // 
   // Initialization
   //
   Path    program;
   
   void    usage();
   int     processArgs( int argc, char** argv,
                        tdrp_override_t& override );
   
   //
   // Parameter processing
   //
   char   *paramsPath;
   Params  params;

   //
   // Messaging
   //
   MsgLog  msgLog;
   void    initLog();

   //
   // Processing
   //
   DataMgr dataMgr;
   
};

//
// Make one instance global
//
#ifdef _RIDDS2DSR_MAIN_
         Ridds2Dsr *ridds2dsr;
#else
  extern Ridds2Dsr *ridds2dsr;
#endif

//
// Macro for easy access to application name
//
#define PROGRAM_NAME ridds2dsr->getProgramName().c_str()


//
// Macros for message logging
//
#define POSTMSG       ridds2dsr->getMsgLog().postMsg
#define DEBUG_ENABLED ridds2dsr->getMsgLog().isEnabled( DEBUG )
#define INFO_ENABLED  ridds2dsr->getMsgLog().isEnabled( INFO )

//
// Defines for success and failure returns
//
#define DSR_FAILURE -1
#define DSR_SUCCESS 0

//
// Prototypes for asyncrhronous handlers
//
void dieGracefully( int signal );

#endif

