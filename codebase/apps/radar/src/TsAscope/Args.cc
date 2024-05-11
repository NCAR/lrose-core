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
//////////////////////////////////////////////////////////

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
      
    } else if (!strcmp(argv[i], "-fmq")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_fmq_url = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = FMQ_INPUT;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-tcp_host")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_tcp_host = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = TCP_INPUT;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
    } else if (!strcmp(argv[i], "-tcp_port")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "input_tcp_port = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = TCP_INPUT;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-simultaneous")) {
      
      sprintf(tmp_str, "simultaneous_mode = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-title")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "main_window_title = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-refresh")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "refresh_hz = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-radar_id")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "radar_id = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-burst_chan")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "burst_chan = %s;", argv[++i]);
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
      
    } else if (!strcmp(argv[i], "-save_dir")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "save_dir = \"%s\";", argv[++i]);
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
      
    } else if (!strcmp(argv[i], "-width")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "main_window_width = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-height")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "main_window_height = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-gate_num")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "start_gate_num = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
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
  out << "TsAscope is the ascope display for time series data in APAR format." << endl;
  out << endl;

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -burst_chan ? ] set burst channel, 0 to 3\n"
      << "       [ -debug, -d ] print debug messages\n"
      << "       [ -fmq ?] set FMQ mode, and input fmq URL\n"
      << "       [ -gate_num ? ] starting gate number for plot\n"
      << "       [ -height ? ] height of main window\n"
      << "       [ -instance ?] set instance for procmap\n"
      << "       [ -radar_id ? ] set radar ID, defaults to 0, i.e. all radars\n"
      << "       [ -refresh ?] set display refresh rate (Hz)\n"
      << "       [ -save_dir ? ] directory for saving images\n"
      << "       [ -simultaneous ] data is simultaneous mode\n"
      << "       [ -start_x ? ] start x location of main window\n"
      << "       [ -start_y ? ] start y location of main window\n"
      << "       [ -tcp_host ?] set TCP server host\n"
      << "       [ -tcp_port ?] set TCP server port\n"
      << "       [ -title ? ] set main window title\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << "       [ -width ? ] width of main window\n"
      << endl;

  Params::usage(out);

}







