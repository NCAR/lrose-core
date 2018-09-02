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
/////////////////////////////////////////////////////////////////////////////
//
// Entry point for application
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  March 1999
//
// $Id: main.cc,v 1.8 2018/09/01 23:55:14 dixon Exp $
//
/////////////////////////////////////////////////////////////////////////////

#include <toolsa/port.h>
#include <toolsa/umisc.h>
#include "Driver.hh"

using namespace std;

static Driver *driver = NULL;
static int exit_count = 0;
static void dieGracefully( int sig );

int main( int argc, char **argv )
{
  int status = 0;
   //        
   // Trap signals for a clean exit
   //                                                                        
   PORTsignal(SIGHUP, dieGracefully );
   PORTsignal( SIGINT,  dieGracefully );                    
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );      
   PORTsignal( SIGKILL, dieGracefully );      
   PORTsignal( SIGPIPE, (PORTsigfunc)SIG_IGN );
   
   //                                                        
   // Instantiate and initialize the top-level application class
   //                              
   driver = new Driver();
   if ( driver->init( argc, argv ) != 0 ) { 
     dieGracefully( status );
   }

   //
   // Startup the application
   //
   status = driver->run();
   
   // exit

   dieGracefully( status );
   return status;
   
}

static void dieGracefully( int sig )
{

  exit_count++;
  if (exit_count == 1) {
    // multiple threads will cause multiple calls to this routine.
    // Therefore the static count is used to prevent freeing the
    // application more than once.
    if (driver) {
      delete driver;
    }
  }
    
  exit( sig );

}

