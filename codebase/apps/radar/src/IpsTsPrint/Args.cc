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
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
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
      
    } else if (!strcmp(argv[i], "-debug")) {
      
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
      
    } else if (!strcmp(argv[i], "-format")) {
      
      sprintf(tmp_str, "print_format = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-once")) {
      
      sprintf(tmp_str, "once_only = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-start")) {
      
      sprintf(tmp_str, "seek_to_end_of_input = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-all_pulses")) {

      sprintf(tmp_str, "print_all_pulses = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-packing")) {
      
      sprintf(tmp_str, "print_packing = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-all_hdrs")) {

      sprintf(tmp_str, "print_all_headers = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-meta_hdrs")) {

      sprintf(tmp_str, "print_meta_headers = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-prt_details")) {

      sprintf(tmp_str, "print_prt_details = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-data")) {

      sprintf(tmp_str, "print_iq_data = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-burst")) {

      sprintf(tmp_str, "print_burst_iq = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "run_mode = PRINT_MODE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-info")) {

      sprintf(tmp_str, "print_info_on_change = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-hv")) {

      sprintf(tmp_str, "print_hv_flag = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-rvp8_legacy")) {

      sprintf(tmp_str, "rvp8_legacy_unpacking = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-cohere")) {

      sprintf(tmp_str, "cohere_iq_to_burst_phase = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-ascope")) {

      sprintf(tmp_str, "run_mode = ASCOPE_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-max_power")) {

      sprintf(tmp_str, "run_mode = MAX_POWER_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-max_power_server")) {

      sprintf(tmp_str, "run_mode = MAX_POWER_SERVER_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-lat_lon")) {

      sprintf(tmp_str, "print_lat_lon = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-altitude")) {
      
      sprintf(tmp_str, "print_altitude = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-velocity")) {
      
      sprintf(tmp_str, "print_radial_velocity = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-server")) {

      sprintf(tmp_str, "run_mode = SERVER_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "server_port = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-cal")) {

      sprintf(tmp_str, "run_mode = CAL_MODE;");
      TDRP_add_override(&override, tmp_str);

      sprintf(tmp_str, "input_mode = TS_FMQ_INPUT;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-dual")) {

      sprintf(tmp_str, "dual_channel = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-alt")) {

      sprintf(tmp_str, "fast_alternating = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-invert")) {
      
      sprintf(tmp_str, "invert_hv_flag = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-missing")) {
      
      sprintf(tmp_str, "print_missing_pulses = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-nsamples")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_samples = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-ngates")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_gates = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-gate")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "start_gate = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-georef_secondary")) {
      
      sprintf(tmp_str, "use_secondary_georeference = true;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-max_angle_change")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_angle_change = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "check_angle_change = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-labels")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "label_interval = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "reg_with_procmap = true;");
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

    } else if (!strcmp(argv[i], "-radar_id")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "radar_id = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "check_radar_id = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-xml_cal")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "cal_xml_file_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "apply_calibration = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if ((!strcmp(argv[i], "-distance_in_ft")) ||
               (!strcmp(argv[i], "-range_in_ft"))) {
      
      sprintf(tmp_str, "distance_units = DISTANCE_IN_FEET;");
      TDRP_add_override(&override, tmp_str);
      
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

    } else {

      tdrpCheckArgAndWarn(argv[i], stderr);
      
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
  out << "IpsTsPrint reads radar time series data, and prints it in various ways. Supports Idependent Pulse Sampling." << endl;
  out << endl;

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -all_pulses ] prints every pulse\n"
      << "       [ -all_hdrs ] prints meta-data and pulse headers\n"
      << "       [ -alt ] fast alternating dual-pol mode\n"
      << "       [ -altitude ] print altitude (max power mode only)\n"
      << "       [ -ascope ] prints data for each gate\n"
      << "       [ -burst ] prints burst IQ data\n"
      << "       [ -cal ] perform calibration\n"
      << "       [ -cohere ] cohere the IQ data to burst phase on read.\n"
      << "         Intended for magnetron systems with random phase\n"
      << "       [ -data ] print iq data\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -distance_in_ft ] Default is to print distance in meters\n"
      << "       [ -dual ] handle dual channels\n"
      << "       [ -f files ] specify input tsarchive file list.\n"
      << "         Read files instead of FMQ.\n"
      << "       [ -fmq ? ] name of input fmq.\n"
      << "       [ -format ] print format of the IPS time series structs\n"
      << "       [ -gate ?] specify start gate number for averaging\n"
      << "       [ -georef_secondary ] use secondary georef\n"
      << "       [ -hv ] prints HV flag in Ascope mode\n"
      << "       [ -info ] prints full info when something changes\n"
      << "       [ -instance ?] instance for registering with procmap\n"
      << "       [ -invert ] invert the sense of the HV flag\n"
      << "       [ -labels ?] number of lines between labels (default 60)\n"
      << "       [ -lat_lon ] prints latitude and longitude (max power mode only)\n"
      << "       [ -max_angle_change ?] set max angle change from pulse to pulse\n"
      << "                              prints warning if change exceeds this\n"
      << "       [ -max_power ] prints max power, and associated range\n"
      << "       [ -max_power_server ] computes max power, runs as server\n"
      << "                             serves out results in XML over TCP/IP\n"
      << "       [ -meta_hdrs ] prints meta-data headers\n"
      << "       [ -missing ] print warning if pulses are not in sequence\n"
      << "       [ -ngates ?] specify number of gates for averaging.\n"
      << "         Defaults to 1.\n"
      << "       [ -nsamples ?] specify number of samples\n"
      << "       [ -once ] print once and exit\n"
      << "       [ -packing ] print data packing: FL32, SCALED_SI16 etc\n"
      << "       [ -port ?] set port for listening in server mode\n"
      << "         Default is 13000.\n"
      << "       [ -prt_details ] prints details about PRT from each pulse\n"
      << "       [ -radar_id ? ] filter on this radar_id\n"
      << "       [ -rvp8_legacy ] RVP8 data is in legacy packing\n"
      << "       [ -start ] read from start of FMQ\n"
      << "       [ -server ] run in server mode\n"
      << "       [ -tcp_host ? ] specify input host for tcp server\n"
      << "       [ -tcp_port ? ] specify input port for tcp server\n"
      << "       [ -velocity ] print radial velocity (max power mode only)\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << "       [ -xml_cal ? ] specify cal xml file path.\n"
      << endl;

  Params::usage(out);

}
