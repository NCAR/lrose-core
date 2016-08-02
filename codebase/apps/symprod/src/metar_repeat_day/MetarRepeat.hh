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
/////////////////////////////////////////////////////////
// metar_repeat_day top-level application class
//
// $Id: MetarRepeat.hh,v 1.2 2016/03/06 23:31:57 dixon Exp $
//
////////////////////////////////////////////////////////
#ifndef _METAR_REPEAT_INC
#define _METAR_REPEAT_INC

#include <string>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include "metar_repeat_day_tdrp.h"

//
// Forward class declarations
//
class MsgLog;
class DataMgr;

class MetarRepeat {
 public:

   MetarRepeat();
   ~MetarRepeat();
   
   //
   // Initialization
   //
   int           init( int argc, char**argv );
   const string& getProgramName(){ return program.getFile(); } 

   //
   // File prefix
   //
   char* getFilePrefix(){ return filePrefix; }

   //
   // Messaging
   //
   MsgLog* getMsgLog() { return msgLog; }
   
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
   int     processArgs( int argc, char **argv,
                        bool *checkParams, bool *printParams,
                        tdrp_override_t& override );
   
   //
   // Parameter processing
   //
   Path    paramPath;
   int     readParams( tdrp_override_t& override,
                       bool checkParams=false,
                       bool printParams=false );
   int     processParams();

   //
   // Messaging
   //
   MsgLog *msgLog;
   void   initLog();

   //
   // Processing
   //
   DataMgr *dataMgr;
   char    *filePrefix;

};

//
// Make one instance global
//
#ifdef _METAR_REPEAT_MAIN_
         MetarRepeat *metarRepeat;
#else
  extern MetarRepeat *metarRepeat;
#endif

//
// Macro for easy access to application name
//
#define PROGRAM_NAME metarRepeat->getProgramName().c_str()

//
// Macros for message logging
//
#define POSTMSG       metarRepeat->getMsgLog()->postMsg
#define DEBUG_ENABLED metarRepeat->getMsgLog()->isEnabled( DEBUG )
#define INFO_ENABLED  metarRepeat->getMsgLog()->isEnabled( INFO )

//
// Macro for file prefix
//
#define FILE_PRFX     metarRepeat->getFilePrefix()

//
// Defines for success and failure returns
//
#define FAILURE -1
#define SUCCESS 0

//
// Prototypes for asyncrhronous handlers
//
void dieGracefully( int signal );

//
// Prototypes for PORTscan select function
//
extern int fileSelect(const struct dirent *dirInfo);

#endif








