/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
//
// main for Rvp8Driver
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#include "Rvp8Driver.hh"
#include <csignal>
#include <cstdlib>
#include <new>
using namespace std;

// file scope

void tidy_and_exit (int sig);
static void out_of_store();
static Rvp8Driver *_prog;
static int _argc;
static char **_argv;
static int exit_count = 0;

// main

int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;
  
  // create program object
  
  _prog = new Rvp8Driver(argc, argv);
  if (!_prog->OK) {
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

void tidy_and_exit (int sig)

{

  exit_count++;
  if (exit_count == 1) {
    delete(_prog);
    exit(sig);
  }

}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  cerr << "FATAL ERROR - program Rvp8Driver" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}

