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
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
/////////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;
  TDRP_init_override(&override);
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
      
    } else if (!strcmp(argv[i], "-lat")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "lat = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-lon")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "lon = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "egm_path = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
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
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -f ?] egm geoid file path\n"
      << "       [ -lat ?] latitude of desired point\n"
      << "       [ -lon ?] longitude of desired point\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << endl;

  Params::usage(out);

}







