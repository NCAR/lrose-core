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
// main for StormShapeSim
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2021
//
///////////////////////////////////////////////////////////////
// StormShapeSim simulates storm shapes and writes these to a
// Cartesian MDV file. It then resamples the Cartesian file
// using a prescribed radar scan strategy, and writes the
// radar-bases simulation to a CfRadial file.
///////////////////////////////////////////////////////////////

#include "StormShapeSim.hh"
#include <toolsa/str.h>
#include <cstdlib>

// file scope

static void tidy_and_exit (int sig);
static StormShapeSim *Prog = NULL;

// main

int main(int argc, char **argv)

{

  // create program object

  Prog = new StormShapeSim(argc, argv);
  if (!Prog->OK) {
    return(-1);
  }

  // run it

  int iret = Prog->Run();

  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

// tidy up on exit

static void tidy_and_exit (int sig)

{
  if (Prog) {
    delete Prog;
    Prog = NULL;
  }
  exit(sig);
}
