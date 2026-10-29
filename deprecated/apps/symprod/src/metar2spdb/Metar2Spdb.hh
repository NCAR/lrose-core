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
// metar2spdb top-level application class
//
////////////////////////////////////////////////////////
#ifndef _METAR_SPDB_INC
#define _METAR_SPDB_INC

#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class MsgLog;
class DataMgr;

class Metar2Spdb {
 public:

   Metar2Spdb();
   ~Metar2Spdb();
   
   //
   // Initialization
   //
   int init( int argc, char**argv );
   const string& getProgramName(){ return program.getFile(); }

   //
   // File prefix
   //
  void  setPrfx(char* prefix);
  char *getPrfx() { return inputFilePrfx; }

  // get time from file name

  void GetTimeFromFileName(char *FileName, time_t *time);

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
   Params _params;
   
   void    usage();
   int     processArgs( int argc, char **argv,
                        tdrp_override_t& override );
   
   //
   // Parameter processing
   //
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
   char    *inputFilePrfx;
   char    *SingleFile; // The filename specified with the -f option
   time_t   SingleTime; // The time decoded from that file.

};

//
// Make one instance global
//
#ifdef _METARMAIN_
         Metar2Spdb *metar2spdb;
#else
  extern Metar2Spdb *metar2spdb;
#endif

//
// Macro for easy access to application name
//
#define PROGRAM_NAME metar2spdb->getProgramName().c_str()

//
// Macros for message logging
//
#define POSTMSG       metar2spdb->getMsgLog()->postMsg
#define DEBUG_ENABLED metar2spdb->getMsgLog()->isEnabled( DEBUG )
#define INFO_ENABLED  metar2spdb->getMsgLog()->isEnabled( INFO )

//
// Macros for easy access to file prefix string
//
#define FILE_PRFX metar2spdb->getPrfx()

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








