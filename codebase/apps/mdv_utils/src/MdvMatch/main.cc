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
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
// $Id: main.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/port.h>
#include <toolsa/umisc.h>

#define _MDVMATCH_MAIN_
#include "Driver.hh"
using namespace std;

int main( int argc, char **argv )
{
   int status;

   //        
   // Trap signals for a clean exit
   //                                                                        
   PORTsignal( SIGHUP, dieGracefully );
   PORTsignal( SIGINT,  dieGracefully );                    
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );      
   PORTsignal( SIGKILL, dieGracefully );      
   PORTsignal( SIGPIPE, (PORTsigfunc)SIG_IGN );
   
   //                                                        
   // Instantiate and initialize the top-level application class
   //                              
   driver = new Driver();
   status = driver->init( argc, argv );
   if ( status != 0 ) { 
      POSTMSG( ERROR, "Application initialization failed." );
      dieGracefully( status );
   }

   //
   // Startup the application -- the end.
   //
   status = driver->run();
   dieGracefully( status );
}
