#include "SpdbSim.hh"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

static SpdbSim *Prog = NULL;

static void request_stop(int)
{
  // The handler only changes signal-safe state; cleanup stays on main.
  if (Prog != NULL) {
    Prog->requestStop();
  }
}

static void out_of_store()
{
  cerr << "FATAL ERROR - SpdbSim ran out of memory" << endl;
  exit(1);
}

int main(int argc, char **argv)
{
  Prog = new SpdbSim(argc, argv);
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

