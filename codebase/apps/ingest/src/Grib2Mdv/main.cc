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
// Main entry point for Grib2Mdv application
///////////////////////////////////////////////////////

// C++ include files
#include <csignal>
#include <cstdlib>

// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "Grib2Mdv.hh"
using namespace std;

void dieGracefully( int signal );

static Grib2Mdv *mainObj = 0;


int
main( int argc, char **argv )
{
   int iret = 0;

   //
   // Instantiate and initialize the top-level application class
   //
   mainObj = Grib2Mdv::instance(argc, argv);
   if ( !mainObj->isOK() ) {
      dieGracefully( -1 );
   }

   //
   // Trap signals for a clean exit
   //
   PORTsignal( SIGHUP,  dieGracefully );
   PORTsignal( SIGINT,  dieGracefully );
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );
   PORTsignal( SIGKILL, dieGracefully );
   PORTsignal( SIGPIPE, (PORTsigfunc)SIG_IGN );

   //
   // Fire off the application
   //
    iret = 0;
    
    if (!mainObj->run()) {
      cerr << mainObj->getErrStr() << endl;
      iret = 1;
    }

   dieGracefully( iret );
}


void dieGracefully( int signal )
{
   //
   // Remove the top-level application class
   //
   delete mainObj;
   exit( signal );
}
