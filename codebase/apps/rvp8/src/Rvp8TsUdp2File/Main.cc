// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:20 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
//
// main driver for Rvp8TsUdp2File
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Rvp8TsUdp2File reads data from UDP in RVP8 TimeSeries format and
// writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#include "Rvp8TsUdp2File.hh"
#include <csignal>
#include <new>
#include <cstdio>
using namespace std;

// file scope

static void tidy_and_exit(int sig);
static void out_of_store();
static Rvp8TsUdp2File *_prog;

// main

int main(int argc, char **argv)
  
{

  // create program object

  _prog = new Rvp8TsUdp2File(argc, argv);
  if (!_prog->isOK) {
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

  int iret = _prog->Run();

  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  delete(_prog);
  exit(sig);

}
////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  fprintf(stderr, "FATAL ERROR - program Rvp8TsUdp2File\n");
  fprintf(stderr, "  Operator new failed - out of store\n");
  exit(-1);

}
