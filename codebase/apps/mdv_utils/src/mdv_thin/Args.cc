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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Args.cc,v 1.8 2016/03/04 02:22:15 dixon Exp $
 *
 */

//////////////////////////////////////////////////////////
//
// Class:	Args
//
// Author:	G. M. Cunning
//
// Date:	Sat Feb  5 15:30:50 2000
//
// Description: This class handles the command line arguments
//
//
//
//
//
//

#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

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

  TDRP_init_override(&override);

  // search for command options
  
  for(int i = 1; i < argc; i++) {

    if(STRequal_exact(argv[i], "--") ||
       STRequal_exact(argv[i], "-h") ||
       STRequal_exact(argv[i], "-help") ||
       STRequal_exact(argv[i], "-man")) {
      _usage(prog_name, cout);
      exit(0);
    }
    else if(STRequal_exact( argv[i], "-debug")) {
      _setOverride("debug = TRUE;");
    }
    else if(STRequal_exact(argv[i], "-i")) {
      if (i < argc - 1) {
	_setOverride("instance", argv[++i]);
      }
      else {
	cerr << _errStr;
	cerr << "User must provide instance name with -i option" << endl;
	
	isOK = false;
      }
    }
    else if ( STRequal_exact(argv[i], "-start") )
    {
      if ( i < argc - 1 ) {
	if ( (_archiveStartTime =
	      DateTime::parseDateTime(argv[++i])) == DateTime::NEVER ) {
	  cerr << _errStr;
	  cerr << "Invalid time string specified with -start option" << endl;
	
	  isOK = false;
	}
      }
      else
      {
	cerr << _errStr;
	cerr << "User must provide time string with -start option" << endl;
	
	isOK = false;
      }
    }
    else if ( STRequal_exact( argv[i], "-end" ) ) {
      if ( i < argc - 1 ) {
	if ( (_archiveEndTime =
	      DateTime::parseDateTime(argv[++i])) == DateTime::NEVER ) {
	  cerr << _errStr;
	  cerr << "Invalid time string specified with -end option" << endl;
	
	  isOK = false;
	}
      }
      else {
	cerr << _errStr;
	cerr << "User must provide time string with -end option" << endl;
	
	isOK = false;
      }
    }
    else if ( STRequal_exact(argv[i], "-mode") ) {
      if ( i < argc - 1 ) {
	_setOverride( "runMode", argv[++i] );
      }
      else {
	cerr << _errStr;
	cerr << "User must provide REALTIME, ARCHIVE, INTERVAL_REALTIME or INTERVAL_ARCHIVE with -mode option" <<
	  endl;
	
	isOK = false;
      }
    }
    else if ( STRequal_exact(argv[i], "-interval") ) {
      if ( i < argc - 1 ) {
	_setOverride( "intervalSecs", argv[++i] );
      }
      else {
	cerr << _errStr;
	cerr << "User must provide number of seconds with -interval option" <<
	  endl;
	
	isOK = false;
      }
    }
    else if ( STRequal_exact(argv[i], "-interval_start") ) {
      if ( i < argc - 1 ) {
	_setOverride( "intervalStartSecs", argv[++i] );
      }
      else {
	cerr << _errStr;
	cerr << "User must provide number of seconds with -interval_start option" <<
	  endl;
	
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
      << "       [-i instance_name] Instance string (no blanks)\n"
      << "       [-mode operational_mode] ARCHIVE, REALTIME, INTERVAL_REALTIME or INTERVAL_ARCHIVE\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "                                 ARCHIVE or INTERVAL_ARCHIVE mode only\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "                             ARCHIVE or INTERVAL_ARCHIVE mode only\n"
      << "       [-i instance_name] Instance string (no blanks)\n"
      << "       [-interval interval_secs] Number of seconds between interval triggers\n"
      << "                             INTERVAL_REALTIME or INTERVAL_ARCHIVE mode only\n"
      << "       [-interval_start interval_start_secs] Number of seconds after the hour for first trigger\n"
      << "                             INTERVAL_REALTIME mode only\n"
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
