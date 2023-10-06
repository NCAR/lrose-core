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
///////////////////////////////////////////////////////////////
//
// main for Era5Nc2Mdv
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2023
//
///////////////////////////////////////////////////////////////
//
// Era5Nc2Mdv ingests ERA5 data and converts to MDV format.
// The ERA5 files are in NetCDF, created using NCAR/CISL software
// that sub-sections a part of the global data set.
//
////////////////////////////////////////////////////////////////

#include "Era5Nc2Mdv.hh"
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <signal.h>
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static Era5Nc2Mdv *Prog;

// main

int main(int argc, char **argv)
{
  // set signal handling

  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // create program object

  Prog = new Era5Nc2Mdv();
  if (!Prog->init(argc, argv)) {
    return -1;
  }

  // run it

  int iret = 0;

  if (Prog->Run()) {
    iret = -1;
  }

  // clean up
  
  tidy_and_exit(iret);
  return iret;
}

// tidy up on exit

static void tidy_and_exit (int sig)
  
{
  delete Prog;
  exit(sig);
}



