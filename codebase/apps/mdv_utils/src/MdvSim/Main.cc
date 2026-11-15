// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD license applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that the
// ** copyright notice, conditions, and disclaimer are retained.
// ** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include "MdvSim.hh"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

static MdvSim *Prog = NULL;

static void request_stop(int)
{
  // The handler only changes signal-safe state; cleanup stays on main.
  if (Prog != NULL) {
    Prog->requestStop();
  }
}

static void out_of_store()
{
  cerr << "FATAL ERROR - MdvSim ran out of memory" << endl;
  exit(1);
}

int main(int argc, char **argv)
{
  Prog = new MdvSim(argc, argv);
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
