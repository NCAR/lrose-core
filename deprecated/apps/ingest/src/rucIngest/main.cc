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
///////////////////////////////////////////////////////
// Main entry point for rucIngest application
//
// $Id: main.cc,v 1.10 2016/03/07 01:23:11 dixon Exp $
//
///////////////////////////////////////////////////////

// C++ include files
#include <csignal>

// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "RucIngest.hh"
using namespace std;

void dieGracefully( int signal );

static RucIngest *mainObj = 0;


int
main( int argc, char **argv )
{
   //
   // Instantiate and initialize the top-level application class
   //
   mainObj = RucIngest::instance(argc, argv);
   if ( !mainObj->isOK() ) {
      dieGracefully( -1 );
   }

   //
   // Trap signals for a clean exit
   //
   PORTsignal( SIGINT,  dieGracefully );
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );
   PORTsignal( SIGKILL, dieGracefully );

   //
   // Fire off the application
   //
  bool iret = mainObj->run();
  if ( !iret ) {
    cerr << mainObj->getErrStr() << endl;
    dieGracefully( 1 );
  }

  dieGracefully( 0 );
}


void dieGracefully( int signal )
{
   //
   // Remove the top-level application class
   //
   delete mainObj;
   exit( signal );
}
