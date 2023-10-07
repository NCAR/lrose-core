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
////////////////////////////////////////////////////////////////////
// Main entry point for Era5Nc2Mdv application
//
// Converts Grib2 files into MDV format
// Tested GRIB2 Models:
//    gfs004    (gfs half degree resolution)  
//    dgex218   (Downscaled Gfs with Eta Extensions, 10km resolution)
//    eta218    (Eta/Nam 10km resolution)
//    NDFD      (National Digital Forecast Database CONUS operational fields)
//
// -Jason Craig-  Jun 2006
////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <signal.h>
#include <toolsa/port.h>

#include "Era5Nc2Mdv.hh"
#include "Args.hh"
using namespace std;

static void dieGracefully( int signal );

// Global program object
Era5Nc2Mdv *Prog = (Era5Nc2Mdv *)NULL;


//*********************************************************************
int main( int argc, char **argv )
{

   // Create program object by requesting a instance
   Prog = Era5Nc2Mdv::Inst(argc, argv);
   if (!Prog->okay)
     return(-1);

   //
   // Trap signals for a clean exit
   //
   PORTsignal( SIGINT,  dieGracefully );
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );
   PORTsignal( SIGKILL, dieGracefully );

   Prog->run();

   delete Prog;

   return (0);
}


void dieGracefully( int signal )
{
  // Delete the program object.

  //if (Prog != (Era5Nc2Mdv *)NULL)
  //delete Prog;

  exit( signal );
}
