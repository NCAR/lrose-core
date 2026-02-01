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
/////////////////////////////////////////////////////////////
// RefractCalib Main
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2026
//
/////////////////////////////////////////////////////////////
//
// RefractCalib:
//    (a) reads radar scan files, in polar coordinates
//    (b) identifies suitable clutter targets
//    (c) computes the mean phase of those targets for a baseline calibration
//    (d) writes the calibration details to a file.
// Typically we use 6 hours of scans for this purpose.
// Ideally the moisture field should be uniform for this procedure to work well.
//
//////////////////////////////////////////////////////////////

#include "RefractCalib.hh"
#include <cstdio>
#include <toolsa/port.h>
#include <toolsa/umisc.h>

// Prototypes for static functions

static void tidy_and_exit(int sig);

// Global variables

RefractCalib *Prog = nullptr;

//---------------------------------------------------------------------------
int main(int argc, char **argv)
{

  // Create program object.

  Prog = new RefractCalib(argc, argv);
  if (!Prog->okay) {
    return -1;
  }

  // Register function to trap termination and interrupt signals

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);

  // Run the program.

  Prog->run();
  
  // clean up

  tidy_and_exit(0);

  return 0;

}

//---------------------------------------------------------------------------
static void tidy_and_exit(int sig)
{
  exit(sig);
}
