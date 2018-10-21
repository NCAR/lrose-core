/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 2008 UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** $Date: 2018/10/18 18:28:56 $
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Args.cc,v 1.4 2018/10/18 18:28:56 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Args
//
// Author:	G. M. Cunning
//
// Date:	Sat Oct 18 10:04:52 2008
//
// Description: This class handles the command line arguments
//              for AwosSimulate
//
//
//
//
//

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <cstring>
#include <cstdlib>

#include "Args.hh"
#include "Params.hh"

using namespace std;

// define any constants
const string Args::_className = "Args";



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Args::Args( int argc, char **argv, const string& prog_name ) : 
  isOK(true),
  _errStr("")
{

  if(argc < 2) {
    _usage(prog_name, cout);
    exit(0);
  }
    
  TDRP_init_override(&override);

  // search for command options
  
  for(int i = 1; i < argc; i++) {

    if(STRequal_exact(argv[i], "--") ||
       STRequal_exact(argv[i], "-h") ||
       STRequal_exact(argv[i], "-help") ||
       STRequal_exact(argv[i], "--help") ||
       STRequal_exact(argv[i], "-man")) {
      _usage(prog_name, cout);
      exit(0);
    }
    else if(STRequal_exact( argv[i], "-debug")) {
      _setOverride("debug = DEBUG_NORM;");
    }
    else if(STRequal_exact( argv[i], "-verbose")) {
      _setOverride("debug = DEBUG_VERBOSE;");
    }
    else if(STRequal_exact(argv[i], "-i")) {
      if (i < argc - 1) {
	_setOverride("instance", argv[i+1]);
      }
      else {
	cerr << _errStr;
	cerr << "User must provide instance name with -i option" << endl;
	
	isOK = false;
      }
    }
    else if (!strcmp(argv[i], "-f")) {
      
      _setOverride("trigger_mode = FILE_LIST;");
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    _inputFileList.push_back(argv[j]);
	  }
	}
      } else {
	isOK = false;
      }

    }
    else if (!strcmp(argv[i], "-out_url")) {
      if(i < argc - 1) {
	_setOverride( "output_url", argv[i+1] );
      }
      else {
	isOK = false;
      }
    }
  } /* endfor - i */

  if(!isOK) {
    _usage(prog_name, cerr);
  }
    
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Args::~Args()
{
  TDRP_free_override(&override);

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
// _usage() - Print the usage for this program.
//

void Args::_usage(const string& prog_name, ostream &out)
{
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -help, -h, -man ] produce this list.\n"
      << "       [ -debug ] debugging on\n"
      << "       [ -verbose ] verbose debugging on\n"
      << "       [-i instance_name] Instance string (no blanks)\n"
      << "       [ -f files ] specify input file list.\n"
      << "         forces FILELIST mode\n"
      << "       [ -out_url url] Output URL\n"
      << endl << endl;

  Params::usage(out);
}

//////////////////////////////////////////////////
// _setOverride
//
// this function sets TDRP overrides
//
void Args::_setOverride(const string& ov_str)
{
  TDRP_add_override(&override, (char *)ov_str.c_str());
}

void Args::_setOverride(const string& ov_str, const char* val_str)
{
  string tmp_str = ov_str + " = \"" + val_str + "\";";
  TDRP_add_override(&override, (char *)tmp_str.c_str());
}
