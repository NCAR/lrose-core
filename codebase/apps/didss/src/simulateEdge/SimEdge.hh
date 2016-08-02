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
/////////////////////////////////////////////
// $Id: SimEdge.hh,v 1.4 2016/03/06 23:53:43 dixon Exp $
//
// Top level application class
////////////////////////////////////////////

#ifndef _SIM_EDGE_
#define _SIM_EDGE_

#include <tdrp/tdrp.h>
#include <toolsa/Path.hh>
#include "Params.hh"
#include "DataMgr.hh"
using namespace std;

//
// Forward class declarations
//
class Path;
class MsgLog;

class SimEdge 
{
  public:
   
   SimEdge();
   ~SimEdge();

   //
   // Initialization
   //
   int init( int argc, char **argv );
   inline const string& getProgramName(){ return program.getFile(); }


   //
   // Messaging
   //
   MsgLog*             getMsgLog(){ return msgLog; }
   
   //
   // Run it
   //
   int run();
   
  private:

   //
   // Initialization
   //
   Path      program;
   void      usage();
   int       processArgs( int argc, char **argv,
                          tdrp_override_t& override );
   
   //
   // Parameters
   //
   char     *paramsPath;
   Params   *params;
   int       processParams();

   //
   // Messaging
   //
   MsgLog   *msgLog;
   void      initLog();

   //
   // Processing
   //
   DataMgr   *dataMgr;

};

//
// Make one instance global
//
#ifdef _SIM_EDGE_MAIN_
         SimEdge *simEdge;
#else
  extern SimEdge *simEdge;
#endif

//
// Macro for easy access to application name
//
#define PROGRAM_NAME  simEdge->getProgramName().c_str()

//
// Macros for message logging
//
#define POSTMSG       simEdge->getMsgLog()->postMsg
#define DEBUG_ENABLED simEdge->getMsgLog()->isEnabled( DEBUG )
#define INFO_ENABLED  simEdge->getMsgLog()->isEnabled( INFO )

//
// Defines for success and failure returns
//
#define FAILURE -1
#define SUCCESS 0

//
// Prototypes for asyncrhronous handlers
//
void dieGracefully( int status );

#endif

   
   
   
