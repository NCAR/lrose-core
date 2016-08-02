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
////////////////////////////////////////////////////////////////////////////////
//
//  LtgSpdb2GenPt top-level application class
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LTGSPDB2MDV_INC
#define LTGSPDB2MDV_INC

#include <string>
#include <math.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>
#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

class LtgSpdb2GenPt 
{
public:
  
  LtgSpdb2GenPt();
  
  ~LtgSpdb2GenPt();
  
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
  Params             params;
  char              *paramPath;
  tdrp_override_t    tdrpOverride;
  
  int                readParams( int argc, char **argv );
  int                processParams();
  
  //
  // Messaging
  //
  MsgLog             msgLog;

   //
   // Data management
   //
   DataMgr            dataMgr; 
  
};


//
// Make one instance global
//
#ifdef _MAIN_
LtgSpdb2GenPt *ltgSpdb2GenPt;
#else
extern LtgSpdb2GenPt *ltgSpdb2GenPt;
#endif

// 
// Macros for message logging 
// 
#define POSTMSG       ltgSpdb2GenPt->getMsgLog().postMsg
#define DEBUG_ENABLED ltgSpdb2GenPt->getMsgLog().isEnabled( DEBUG )
#define INFO_ENABLED  ltgSpdb2GenPt->getMsgLog().isEnabled( INFO )

//
// Macro for easy access to application name
//
#define PROGRAM_NAME ltgSpdb2GenPt->getProgramName().c_str()

//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif



