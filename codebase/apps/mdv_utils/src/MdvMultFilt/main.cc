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
#include <signal.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>

#include "UrlWatcher.hh"
using namespace std;

//
// Function needed for clean exit.
//
void cleanExit( int signal );

int main( int argc, char **argv )
{
   int status;

   //
   // Trap signals for a clean exit
   //
   PORTsignal( SIGINT,  cleanExit );
   PORTsignal( SIGTERM, cleanExit );
   PORTsignal( SIGQUIT, cleanExit );
   PORTsignal( SIGKILL, cleanExit );

   
   //
   // Instantiate and initialize the top-level application class
   //
   UrlWatcher U;
   status = U.init( argc, argv);
   if ( status != 0 ) {
      cleanExit( status );
   }

   //
   // Fire off the application -- that's it
   //
   status = U.run( );
   cleanExit( status );
  
   return 0;

}

//
/////////////////////////////////////////////////////
//

void cleanExit( int signal )
{
   //
   // Unregister with process mapper and exit
   //
   PMU_auto_unregister();
   exit( signal );
}              












