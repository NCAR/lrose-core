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
#ifndef _Image2Mdv_INC
#define _Image2Mdv_INC

#include <string>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh> 

#include <X11/Xlib.h>
// Imlib version 2
#include <Imlib2.h>

#include "Params.hh"

class Image2Mdv
{
public:
   // Basic Constructor
   Image2Mdv() {};

  // Basic Destructor
  ~Image2Mdv() {};

   // Initialization
   int init(int argc, char **argv );

   // Convienience/ Convention
   static const string version;
   const string&  getProgramName(){ return program.getFile(); }

   // Functional Entry - After Init and Parameter loading
   void run();   // Complete the Application's task

   // Process one file
   void process_file(const char *fname);

   // Message logger
   MsgLog   msgLog;

private:

   //  	Argument Processing
   Path  program;   // The Name of the  Application We're running
   void  usage();
   int   processArgs( int argc, char **argv, char **paramPath);

   // Parameter objects and  processing
   Params params;
   int  checkParamPath( const string &path );
   int  processParams( const string &paramPath );

   string Input_url;  // Pulled from the Arguments
   string Output_fname;  // Pulled from the Arguments
};

//
// Make one instance global
//
#ifdef _MAIN_
   Image2Mdv    *worker;
#else
   extern Image2Mdv  *worker;
#endif

//
// Macro for easy access to application name
//
//#define PROGRAM_NAME worker->getProgramName().c_str()
#define PROGRAM_NAME "Image2Mdv"

//
// Macros for message logging
//
#define POSTMSG       worker->msgLog.postMsg
#define DEBUG_ENABLED worker->msgLog.isEnabled( DEBUG )
#define INFO_ENABLED  worker->msgLog.isEnabled( INFO )
          
//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
