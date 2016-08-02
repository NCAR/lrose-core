// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:31:59 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
//
// main for Rvp8TsTcpServer
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Rvp8TsTcpServer serves out RVP8 time-series data under the 
// TCP/IP protocol.
//
// Rvp8TsTcpServer listens on the specified port for a connection
// from a client. When a client connects, the server starts
// sending time-series data to the client, using the message
// format from the Socket class.
//
// Only a single client may connect at a time.
//
///////////////////////////////////////////////////////////////

#include "Rvp8TsTcpServer.hh"
#include <csignal>
#include <cstdlib>
#include <new>
using namespace std;

// file scope

void tidy_and_exit (int sig);
static void out_of_store();
static Rvp8TsTcpServer *_prog;
static int _argc;
static char **_argv;
static int exit_count = 0;

// main

int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;
  
  // create program object
  
  _prog = new Rvp8TsTcpServer(argc, argv);
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

  cerr << "FATAL ERROR - program Rvp8TsTcpServer" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}

