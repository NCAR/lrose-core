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

#ifndef _DrawFmq2Bdry_INC
#define _DrawFmq2Bdry_INC

#include <string>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh> 
#include <Fmq/DrawQueue.hh> 

class DrawFmq2Bdry
{
public:
   DrawFmq2Bdry();
  ~DrawFmq2Bdry();

   //
   // Initialization
   //
   int init(int argc, char **argv );
   static const string version;
   const string&       getProgramName(){ return program.getFile(); }

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
   void               usage();
   int                processArgs( int argc, char **argv,
                                   string &paramPath,
                                   bool *checkParams, bool *printParams );
   //
   // Parameter processing
   //
   int                checkParamPath( const string &path );
   int                readParams( const string &paramPath, 
                                  bool checkParams, bool printParams );
   int                processParams( const string &paramPath );

   //
   // Input Draw Queue
   // 
   DrawQueue    drawqueue;

};

//
// Make one instance global
//
#ifdef _DFBMAIN_
   DrawFmq2Bdry    *translator;
#else
   extern DrawFmq2Bdry    *translator;
#endif

//
// Macro for easy access to application name
//
//#define PROGRAM_NAME translator->getProgramName().c_str()
#define PROGRAM_NAME "DrawFmq2Bdry"

//
// Macros for message logging
//
#define POSTMSG       translator->msgLog.postMsg
#define DEBUG_ENABLED translator->msgLog.isEnabled( DEBUG )
#define INFO_ENABLED  translator->msgLog.isEnabled( INFO )
          
//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
