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
// $Id: main.cc,v 1.3 2016/03/07 01:23:01 dixon Exp $
//

#include <toolsa/port.h>
#include <toolsa/umisc.h>
#include <toolsa/MsgLog.hh>

#define _MAIN_

#include "Image2Mdv.hh"

int main( int argc, char **argv )
{
   //        
   // Trap signals for a clean exit
   //                                                                        
   PORTsignal( SIGINT,  dieGracefully );                    
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );      
   PORTsignal( SIGKILL, dieGracefully );      
   
   //                                                        
   // Instantiate and initialize the top-level application class
   //                              
   worker = new Image2Mdv;

   if(worker->init(argc, argv) != 0) {
       fprintf(stderr,"Image2Mdv failed to initialize properly\n");
       exit(-1);
    }

   //
   // Startup the application 
   //
   worker->run();

   //
   // We shouldn't ever get here
   //
   POSTMSG( FATAL, "Unexpected exit." );
   dieGracefully( -1 );
}
