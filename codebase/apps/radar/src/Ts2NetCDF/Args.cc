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
// Args.cc
//
// Command line args
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2011
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <toolsa/umisc.h>
using namespace std;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
}

// destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[256];

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
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
      
    } else if (!strcmp(argv[i], "-from_start")) {
      
      sprintf(tmp_str, "seek_to_end_of_input = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rvp8_legacy")) {

      sprintf(tmp_str, "rvp8_legacy_unpacking = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-second")) {
      
      sprintf(tmp_str, "save_second_geometry = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-one_file_only")) {
      
      sprintf(tmp_str, "one_file_only = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-preserve_fname")) {
      
      sprintf(tmp_str, "preserve_file_name = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "save_one_file_per_input_file = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "register_with_procmap = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-input_dir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = TS_ARCHIVE_INPUT;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-fmq")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_fmq_name = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TS_FMQ_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-sectors")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_sector_size = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "save_scans_in_sectors = TRUE;");
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "save_one_file_per_input_file = FALSE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-n_gates_save")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_gates_save = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "specify_n_gates_save = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-pad_n_gates_to_max")) {
      
      sprintf(tmp_str, "pad_n_gates_to_max = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-max_pulses")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_pulses_per_file = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "save_one_file_per_input_file = FALSE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-outdir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-tcp_host")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "tcp_server_host = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TS_TCP_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-tcp_port")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "tcp_server_port = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TS_TCP_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        char *timeArg = argv[++i];
	startTime.set(timeArg);
	if (!startTime.isValid()) {
	  iret = -1;
	} else {
	  sprintf(tmp_str, "input_mode = TS_ARCHIVE_INPUT;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        char *timeArg = argv[++i];
	endTime.set(timeArg);
	if (!endTime.isValid())	{
	  iret = -1;
	} else {
	  sprintf(tmp_str, "input_mode = TS_ARCHIVE_INPUT;");
	  TDRP_add_override(&override, tmp_str);
	}
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
      } else {
	iret = -1;
      }

      if (inputFileList.size() < 1) {
        cerr << "ERROR - with -f you must specify files to be read" << endl;
        iret = -1;
      } else {
	sprintf(tmp_str, "input_mode = TS_FILE_INPUT;");
	TDRP_add_override(&override, tmp_str);
      }

    } else if (argv[i][0] == '-') {

      cerr<< "====>> WARNING - invalid command line argument: '"
          << argv[i] << "' <<====" << endl;

    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << endl;
  out << "Ts2NetCDF reads radar time series files in IWRF ir TsArchive format, and writes files in netCDF format." << endl;
  out << endl;

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] sets end time\n"
      << "          sets input_mode to TS_ARCHIVE_INPUT\n"
      << "       [ -f files ] specify input tsarchive file list.\n"
      << "         Sets input_mode to TS_FILE_INPUT.\n"
      << "       [ -fmq ? ] name of input fmq.\n"
      << "         Sets input_mode to TS_FMQ_INPUT.\n"
      << "       [ -from_start ] read from start of FMQ\n"
      << "       [ -input_dir ?] sets input_dir for TS_ARCHIVE_INPUT mode\n"
      << "       [ -instance ?] instance for registering with procmap\n"
      << "       [ -max_pulses ? ] limit number of pulses per file)\n"
      << "       [ -n_gates_save ? ] only save out rays\n"
      << "         with this number of gates\n"
      << "       [ -one_file_only ] only save 1 file, then quit.\n"
      << "       [ -outdir ? ] specify output directory.\n"
      << "       [ -pad_ngates_to_max] option to pad the number of gates out\n"
      << "         to the max number of gates in the file.\n"
      << "         Not compatible with n_gates_save\n"
      << "       [ -preserve_fname ] use the same file name on output\n"
      << "         A '.nc' extension will be appended to the file name\n"
      << "       [ -rvp8_legacy ] RVP8 data is in legacy packing\n"
      << "       [ -second ] save data from second geom found.\n"
      << "         By default, first geom is saved.\n"
      << "       [ -sectors ? ] save files in sectors of given width (deg)\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] sets start time\n"
      << "          sets input_mode to TS_ARCHIVE_INPUT\n"
      << "       [ -tcp_host ? ] specify host for tcp server\n"
      << "         Sets input_mode to TS_TCP_INPUT.\n"
      << "       [ -tcp_port ? ] specify port for tcp server\n"
      << "         Sets input_mode to TS_TCP_INPUT.\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;

  out << "The -second option is used to specify that you want to save out\n"
      << "  data from the SECOND beam geometry found.\n"
      << "  Some files contain data with differing PRT and nGates.\n"
      << "  By default, this app will discard the second geom found.\n"
      << "  Use -second to select the second geometry and discard the first.\n"
      << endl;

  Params::usage(out);

}







