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
#include <unistd.h>
#include <cstdlib>

#include <toolsa/pmu.h>
#include <toolsa/ttape.h>
#include <toolsa/MsgLog.hh>

#include "DsFmq2Tape.hh"
using namespace std;

void dieGracefully( int signal )
{
   int tapeId = application->getTapeId();

   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

   //
   // Rewind the tape, if we got the special user-defined signal
   // which is sent via snuff_usr1
   //
   if ( signal == SIGUSR1  &&  tapeId >= 0 ) {
      POSTMSG( DEBUG, "Rewinding and ejecting tape" );
      TTAPE_rewoffl( tapeId );
   }

   //
   // Close the tape device if it's open
   //
   if ( tapeId >= 0 ) {
      close( tapeId );
   }

   // Remove the top-level application class
   //
   delete application;

   exit( signal );
}
