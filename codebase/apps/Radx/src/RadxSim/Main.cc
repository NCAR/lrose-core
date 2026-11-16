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
// Main for RadxSim
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// RadxSim is intended to simulate Radx-type files in a realtime
// scenario.
//
// RadxSim loops through a set of polar CfRadial files,
// modifies the ray times to wall-clock time, and writes the
// modified files to an output dir.
//
///////////////////////////////////////////////////////////////

#include "RadxSim.hh"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

static RadxSim *Prog = NULL;

static void request_stop(int)
{
  // Signal handling only flips a flag; cleanup remains on the main thread.
  if (Prog != NULL) {
    Prog->requestStop();
  }
}

static void out_of_store()
{
  cerr << "FATAL ERROR - RadxSim ran out of memory" << endl;
  exit(1);
}

int main(int argc, char **argv)
{
  Prog = new RadxSim(argc, argv);
  if (!Prog->OK) {
    delete Prog;
    Prog = NULL;
    return -1;
  }

  signal(SIGINT, request_stop);
  signal(SIGHUP, request_stop);
  signal(SIGTERM, request_stop);
  signal(SIGPIPE, SIG_IGN);
  set_new_handler(out_of_store);

  const int iret = Prog->Run();
  delete Prog;
  Prog = NULL;
  return iret;
}

