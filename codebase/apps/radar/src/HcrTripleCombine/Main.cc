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
//////////////////////////////////////////////////////////////////////////
//
// main for HcrTripleCombine
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2026
//
//////////////////////////////////////////////////////////////////////////
//
// HCR has the capability to transmit blocks of pulses with varying PRTs 
//   and pulse lengths.
//
// The latest version supports 3 block types:
//
//      (1) short-pulse and short-PRT
//      (2) long-pulse and long-PRT
//      (3) long-pulse and short-PRT.
//
// This sequence is repeated in time.
//
// HcrTs2Moments reads this interleaved time series, and computes the 
//   relevant moments for each block. Those moments are then written, in 
//   sequence, to a single output FMQ in Radx moments format.
//
// HcrTripleCombine reads the Radx moments data stream, and combines the 
//   three blocks into a single block, naming the fields appropriately, 
//   and unfolding the velocity fields as appropriate. This allows us to 
//   unfold the velocity field using the staggered-PRT technique.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrTripleCombine.hh"
#include <signal.h>
#include <new>
#include <iostream>
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static HcrTripleCombine *Prog = NULL;

// main

int main(int argc, char **argv)

{

  // create program object

  Prog = new HcrTripleCombine(argc, argv);
  if (!Prog->OK) {
    cerr << "Error: Could not create HcrTripleCombine object." << endl;
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
    cerr << "ERROR - running HcrTripleCombine" << endl;
  }
  
  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

// tidy up on exit

static void tidy_and_exit(int sig)

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

  cerr << "FATAL ERROR - program HcrTripleCombine" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(1);

}
