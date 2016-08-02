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
//
// $Id 
//

#ifndef _RemoteUI2Fmq_INC
#define _RemoteUI2Fmq_INC

#include <string>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh> 
#include <Fmq/RemoteUIQueue.hh> 
using namespace std;

class RemoteUI2Fmq
{
public:
   RemoteUI2Fmq();
  ~RemoteUI2Fmq();

   //
   // Initialization
   //
   int init(int argc, char **argv);

   static const string version;

   const string & getProgramName(){ return program.getFile(); }

   void                run();

   //
   // Message logger
   //
   MsgLog            msgLog;

private:

   //
   // Initialization
   //
   Path               program;
   char *        in_fname; // Input filename
   char *        output_fmq_url; // Output FMQ URL 
   bool          debug;

   void               usage();
   int                processArgs( int argc, char **argv);

   // Input Queue
   // 
   RemoteUIQueue    remoteQueue;
};

//
// Make one instance global
//
#ifdef _RUIFMAIN
   RemoteUI2Fmq    *sender;
#else
   extern RemoteUI2Fmq    *sender;
#endif

//
// Macro for easy access to application name
//
//#define PROGRAM_NAME sender->getProgramName().c_str()
#define PROGRAM_NAME "RemoteUI2Fmq"

//
// Macros for message logging
//
#define POSTMSG       sender->msgLog.postMsg
#define DEBUG_ENABLED sender->msgLog.isEnabled( DEBUG )
#define INFO_ENABLED  sender->msgLog.isEnabled( INFO )
          
//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
