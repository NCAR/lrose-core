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
//  Ascii2Mdv top-level application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  June 1999
//
//  $Id: Ascii2Mdv.hh,v 1.5 2016/03/04 02:22:14 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _ASCII2MDV_INC_
#define _ASCII2MDV_INC_

#include <string>
#include <math.h>

#include <toolsa/pmu.h>
#include <toolsa/Path.hh>

#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class MsgLog;


class Ascii2Mdv
{
public:
   Ascii2Mdv();
  ~Ascii2Mdv();

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
   // Data manager
   //                                                    
   DataMgr*            getDataMgr(){ return dataMgr; }                         

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
   Path               paramPath;
   Params             params;
   tdrp_override_t    override;

   int                readParams( int argc, char **argv );
   int                processParams();
   int                checkParamValues();

   //
   // Messaging
   //
   MsgLog            *msgLog;

   //
   // Data management and execution
   //
   DataMgr           *dataMgr;
};

//
// Make one instance global
//
#ifdef _ASCII2MDV_MAIN_
          Ascii2Mdv *ascii2Mdv;
#else
   extern Ascii2Mdv *ascii2Mdv;
#endif

// 
// Macros for message logging 
// 
#define POSTMSG       ascii2Mdv->getMsgLog()->postMsg
#define INFO_ENABLED  ascii2Mdv->getMsgLog()->isEnabled( INFO )
#define DEBUG_ENABLED ascii2Mdv->getMsgLog()->isEnabled( DEBUG )
   
//
// Macro for easy access to application name
//
#define PROGRAM_NAME ascii2Mdv->getProgramName().c_str()

//
// Empty strings
//
#define ISEMPTY(a)  ( a[0] == '\0' )

//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
