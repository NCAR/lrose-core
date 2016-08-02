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
// $Id: DsFmq2Tape.hh,v 1.3 2016/03/06 23:53:39 dixon Exp $
//

#ifndef _DSFMQ2TAPE_INC_
#define _DSFMQ2TAPE_INC_

#include <string>
#include <toolsa/Path.hh>

#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class DsFmq;
class MsgLog;


class DsFmq2Tape
{
public:
   DsFmq2Tape();
  ~DsFmq2Tape();

   //
   // Initialization
   //
   static const string version;
   int                 init( int argc, char **argv );
   const string&       getProgramName(){ return program.getFile(); }

   //
   // Messaging
   //
   MsgLog*             getMsgLog(){ return msgLog; }

   //
   // Fmq and tape management
   //
   void                run();
   int                 getTapeId(){ return tapeId; }


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
   Path               paramPath;
   Params             params;
   int                readParams( int argc, char **argv );
   int                processParams();

   //
   // Message logging
   //
   MsgLog            *msgLog;
   int                initLog();

   //
   // Fmq and tape management
   //
   DsFmq             *fmq;
   int                tapeId;

};

//
// Make one instance global
//
#ifdef _DSFMQ2TAPE_MAIN_
          DsFmq2Tape *application;
#else
   extern DsFmq2Tape *application;
#endif

//
// Macros for message logging
//
#define POSTMSG       application->getMsgLog()->postMsg
#define DEBUG_ENABLED application->getMsgLog()->isEnabled( DEBUG )
#define INFO_ENABLED  application->getMsgLog()->isEnabled( INFO )

//
// Macro for easy access to application name
//
#define PROGRAM_NAME application->getProgramName().c_str()

//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
