/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;

}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (const int argc, const char **argv)

{

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-n_threads")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "n_threads = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-n_vals_per_thread")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "n_vals_per_thread = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-max_val")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "max_val = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-run_once")) {
      
      sprintf(tmp_str, "thread_mode = THREADS_RUN_ONCE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-use_pool")) {
      
      sprintf(tmp_str, "thread_mode = THREAD_POOL;");
      TDRP_add_override(&override, tmp_str);
      
    } // if
    
  } // i

  if (iret) {
    _usage(cerr);
  }

  return (iret);
    
}

void Args::_usage(ostream &out)

{

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] turn on debugging\n"
      << "       [ -n_threads ? ] number of threads\n"
      << "       [ -max_val ? ] set max value for computations\n"
      << "       [ -n_vals_per_thread ? ] number of values per thread\n"
      << "       [ -run_once ] run threads once\n"
      << "       [ -use_pool ] use thread pool\n"
      << "       [ -verbose ] turn on verbose debugging\n"
      << endl;

  Params::usage(out);

}







