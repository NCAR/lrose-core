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
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
///////////////!///////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <cstdlib>
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
      
    } else if (!strcmp(argv[i], "-debug") ||
               !strcmp(argv[i], "-d")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose") ||
               !strcmp(argv[i], "-v")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra") ||
               !strcmp(argv[i], "-vv")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-fmq_mode")) {
      
      sprintf(tmp_str, "input_mode = IWRF_FMQ_INPUT;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-color_scales")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "color_scale_dir = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-fmq_url")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_fmq_url = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "instance = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "register_with_procmap = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-tcp_mode")) {
      
      sprintf(tmp_str, "input_mode = IWRF_TCP_INPUT;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-tcp_host")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_tcp_host = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-tcp_port")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_tcp_port = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start_x")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "main_window_start_x = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start_y")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "main_window_start_y = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start_time")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "archive_start_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = ARCHIVE_TIME_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-time_span")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "archive_time_span_secs = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = ARCHIVE_TIME_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-archive_url")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "archive_data_url = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = ARCHIVE_TIME_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    inputFileList.push_back(argv[j]);
	  }
	}
        sprintf(tmp_str, "input_mode = FILE_LIST_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

      if (inputFileList.size() < 1) {
        cerr << "ERROR - with -f you must specify files to be read" << endl;
        iret = -1;
      }

    } else if (argv[i][0] == '-') {

      cerr<< "====>> WARNING - invalid command line argument: '"
          << argv[i] << "' <<====" << endl;

    } // if
    
  } // i

  if (iret) {
    _usage(cerr);
  }

  return (iret);
    
}

void Args::_usage(ostream &out)

{

  out << endl;
  out << "Sprite is the spectral display for time series data" << endl;
  out << endl;

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -archive_url ? ] URL for data in archive mode\n"
      << "       [ -color_scales ? ] specify color scale directory\n"
      << "       [ -debug, -d ] print debug messages\n"
      << "       [ -f ? ?] list of files to process in archive mode\n"
      << "       [ -fmq_mode ] set IWRF_FMQ_INPUT mode\n"
      << "       [ -fmq_url ? ] set input fmq URL\n"
      << "       [ -instance ? ] set instance for procmap\n"
      << "       [ -start_time \"yyyy mm dd hh mm ss\"]\n"
      << "          set start time for archive mode\n"
      << "       [ -start_x ? ] start x location of main window\n"
      << "       [ -start_y ? ] start y location of main window\n"
      << "       [ -tcp_mode] IWRF_TCP_INPUT mode\n"
      << "       [ -tcp_host ? ] set TCP server host\n"
      << "       [ -tcp_port ? ] set TCP server port\n"
      << "       [ -time_span ?]\n"
      << "          set time span (secs)\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;

  Params::usage(out);

}







