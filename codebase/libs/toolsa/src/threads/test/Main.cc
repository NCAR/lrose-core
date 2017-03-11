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
// main for ThreadTest
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// ThreadTest tests out the C++ mode of TDRP
//
////////////////////////////////////////////////////////////////

#include "ThreadTest.hh"
#include <toolsa/str.h>
#include <cstdlib>

// file scope

static void tidy_and_exit (int sig);
static ThreadTest *Prog;

// main

int main(int argc, char **argv)

{

  // create program object

  ThreadTest *Prog;
  Prog = new ThreadTest(argc, argv);
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
  delete(Prog);
  exit(sig);
}
