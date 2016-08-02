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
 *  $Id: Args.cc,v 1.7 2016/03/07 01:23:05 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: Args.cc,v 1.7 2016/03/07 01:23:05 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Args
//
// Author:	G. M. Cunning
//
// Date:	Tue Apr 17 15:13:02 2007
//
// Description: This class handles the command line arguments.
//

// System/RAP include files
#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
//#include <toolsa/umisc.h>


// Local include files
#include "Args.hh"
#include "Params.hh"

using namespace std;


//
// argument strings
//
static const string ARG_DEBUG = "-debug";
static const string ARG_VERBOSE = "-verbose";
static const string ARG_START_TIME = "-start_time";
static const string ARG_END_TIME = "-end_time";
static const string ARG_IN_FILE_LIST = "-in_file_list";
static const string ARG_IN_DIR_LIST = "-in_dir_list";
static const string ARG_RT_IN_DIR = "-rt_in_dir";
static const string ARG_OUT_URL = "-out_url";
static const string ARG_CONVERT = "-convert";


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Args::Args() : 
  isOK(true),
  _archiveStartTime(0),
  _archiveEndTime(0)
{
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
//
// Method Name:	Args::parse
//
// Description:	parse the argument list
//
// Returns:	none
//
// Notes:	
//
//

void
Args::parse(int argc, char **argv, const string& prog_name)
{

  TDRP_init_override(&override);

  // search for command options
  
  for (int i =  1; i < argc; i++) {

    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "--help") ||
	STRequal_exact(argv[i], "-man")) {
      _usage(prog_name, cout);
      exit(0);
    } 
    else if (STRequal_exact(argv[i], ARG_DEBUG.c_str())) {
      _setOverride("debug_mode = DEBUG_NORM;");
    } 
    else if (STRequal_exact(argv[i], ARG_VERBOSE.c_str())) {
      _setOverride("debug_mode = DEBUG_VERBOSE;");
    } 
    else if (STRequal_exact(argv[i], ARG_CONVERT.c_str())) {
      _setOverride("convert_counts = TRUE;");
    } 
    else if (STRequal_exact(argv[i], "-i")) {
      
      if (i < argc - 1) {
	_setOverride("instance", argv[i+1]);
      }
      else {
	cerr << "User must provide instance name with -i option" << endl;
	
	isOK = false;
      }

    }
    else if (STRequal_exact(argv[i], ARG_START_TIME.c_str()))
    {

      if (i < argc - 1) {
	if ((_archiveStartTime = DateTime::parseDateTime(argv[++i]))
	  == DateTime::NEVER) {
	  cerr << "Invalid time string specified with " << ARG_END_TIME << " option" << endl;
	
	  isOK = false;
	}
	else {
	  _setOverride("mode = TIME_LIST;");
	}
      }
      else
      {
	cerr << "User must provide time string with " << ARG_END_TIME << " option" << endl;
	
	isOK = false;
      }

    } // endif -- (STRequal_exact(argv[i], ARG_START_TIME.c_str()
    else if (STRequal_exact(argv[i], ARG_END_TIME.c_str())) {

      if (i < argc - 1) {
	if ((_archiveEndTime = DateTime::parseDateTime(argv[++i]))
	     == DateTime::NEVER) {
	  cerr << "Invalid time string specified with " << ARG_END_TIME << " option" << endl;
	
	  isOK = false;
	}
	else {
	  _setOverride("mode = TIME_LIST;");
	}
      }
      else {
	cerr << "User must provide time string with " << ARG_END_TIME << " option" << endl;
	
	isOK = false;
      }

    } // endif -- STRequal_exact(argv[i], ARG_END_TIME.c_str())
    else if (STRequal_exact(argv[i], ARG_IN_FILE_LIST.c_str())) {

      if (i < argc - 1) {
        // load up file list vector. Break at next arg which
        // start with -
        for (int j = i + 1; j < argc; j++) {
          if (argv[j][0] == '-') {
            break;
          } 
	  else {
            _inputFileList.push_back(argv[j]);
          } // endif -- argv[j][0] == '-'
        } // endfor -- int j = i + 1; j < argc; j++
        _setOverride("mode = FILE_LIST;");
      } 
      else {
        cerr << "Invalid input file list" << endl;
        isOK = false;
      } // endif -- i < argc - 1

    } // endif -- STRequal_exact(argv[i],  ARG_IN_FILE_LIST.c_str())
    else if (STRequal_exact(argv[i], ARG_IN_DIR_LIST.c_str())) {

      if (i < argc - 1) {
        // load up directory list vector. Break at next arg which
        // start with -
        for (int j = i + 1; j < argc; j++) {
          if (argv[j][0] == '-') {
            break;
          } 
	  else {
            _inputDirList.push_back(argv[j]);
          } // endif -- argv[j][0] == '-'
        } // endfor -- int j = i + 1; j < argc; j++
        _setOverride("mode = DIR_LIST;");
      } 
      else {
        cerr << "Invalid input directory list" << endl;
        isOK = false;
      } // endif -- i < argc - 1

    } // endif -- STRequal_exact(argv[i], ARG_IN_DIR_LIST.c_str())
    else if (STRequal_exact(argv[i], ARG_RT_IN_DIR.c_str())) {

      if (i < argc - 1) {
	_setOverride("input_dir", argv[++i]);
      }
      else {
	cerr << "User must provide a real-time input directory name with -rt_in_dir option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1

    } // endif -- STRequal_exact(argv[i], ) 
    else if(STRequal_exact(argv[i], ARG_OUT_URL.c_str()) || STRequal_exact(argv[i], "-o")) {
      if(i < argc - 1) {
	_setOverride("output_url", argv[++i]);
      }
      else {
	cerr << "User must provide URL or directory name with " << ARG_OUT_URL << " option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1
    } // endif -- STRequal_exact(argv[i],  ARG_OUT_URL.c_str())
  
} // endfor -- int i =  1; i < argc; i++

  if(!isOK) {
    _usage(prog_name, cerr);
  } // endif -- !isOK
    
}

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
      << "       [ " << ARG_DEBUG << " ] debugging on\n"
      << "       [ " << ARG_VERBOSE << " ] verbose debugging on\n"
      << "       [ " << ARG_START_TIME << " \"yyyy mm dd hh mm ss\" ] start time for processing\n"
      << "       [ " << ARG_END_TIME << " \"yyyy mm dd hh mm ss\" ] end time for processing\n"
      << "       [ " << ARG_IN_FILE_LIST << " <file file ...> ] list of files to process\n"
      << "       [ " << ARG_IN_DIR_LIST << " <dir dir ...> ] list of directories to process\n"
      << "       [ " << ARG_RT_IN_DIR << " <dir> ] real-time input directory\n"
      << "       [ " << ARG_OUT_URL << " <url> ] URL or directory for output files\n"
      << "       [ " << ARG_CONVERT << " ] turns on convsersion of bdt. counts to temperatures and albedos\n"
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



