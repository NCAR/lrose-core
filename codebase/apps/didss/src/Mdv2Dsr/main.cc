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
///////////////////////////////////////////////////////////
// main for Mdv2Dsr application
//
// $Id: main.cc,v 1.4 2016/03/06 23:53:41 dixon Exp $
//
//////////////////////////////////////////////////////////
#include <signal.h>
#include <toolsa/port.h>

#define _MDV2DSR_MAIN_
#include "Mdv2Dsr.hh"
using namespace std;

int main( int argc, char **argv )
{
   int status;
   
   //
   // Register function to trap termination and interrupts
   //
   PORTsignal(SIGTERM, dieGracefully);
   PORTsignal(SIGINT, dieGracefully);
   PORTsignal(SIGQUIT, dieGracefully);
   PORTsignal(SIGKILL, dieGracefully);
   PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

   //
   // Instantiate and initialize top level application class
   //
   mdv2dsr = new Mdv2Dsr();
   status = mdv2dsr->init( argc, argv );
   if( status != SUCCESS ) {
      dieGracefully( status );
   }
   
   //
   // Fire off the application
   //
   status = mdv2dsr->run();

   //
   // Exit
   //
   dieGracefully( status );

}
