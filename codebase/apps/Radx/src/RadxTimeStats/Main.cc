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
// main for RadxTimeStats
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2023
//
///////////////////////////////////////////////////////////////
//
// RadxTimeStats identifies persistent clutter in polar radar data,
// flags it, and writes out the statistics to a CfRadial file.
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Based on following paper:
// Lakshmanan V., J. Zhang, K. Hondl and C. Langston.
// A Statistical Approach to Mitigating Persistent Clutter in
// Radar Reflectivity Data.
// IEEE Journal of Selected Topics in Applied Earth Observations
// and Remote Sensing, Vol. 5, No. 2, April 2012.
////////////////////////////////////////////////////////////////

#include "RadxTimeStats.hh"
#include <signal.h>
#include <new>
#include <iostream>
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static RadxTimeStats *Prog = NULL;

// main

int main(int argc, char **argv)

{

  // create program object

  Prog = new RadxTimeStats(argc, argv);
  if (!Prog->OK) {
    cerr << "Error: Could not create RadxTimeStats object." << endl;
    return(-1);
  }

  // set signal handling
  
  signal(SIGINT, tidy_and_exit);
  signal(SIGHUP, tidy_and_exit);
  signal(SIGTERM, tidy_and_exit);
  signal(SIGPIPE, SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = Prog->Run();
  if (iret < 0) {
    cerr << "ERROR - running RadxTimeStats" << endl;
  }
  
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

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  cerr << "FATAL ERROR - program RadxTimeStats" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}
