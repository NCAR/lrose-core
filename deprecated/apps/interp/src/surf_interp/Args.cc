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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
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

int Args::parse(int argc, char **argv, const string &prog_name)

{

  int iret = 0;
  char tmp_str[BUFSIZ];

  // intialize

  TR_spec = false;
  startTime = 0;
  endTime = 0;
  TDRP_init_override(&override);

  bool start_spec=FALSE;
  bool end_spec=FALSE;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-latlon")) {
      
      sprintf(tmp_str, "flat = 0;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-interval")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_trigger_interval = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-mode")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-minstations")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "MinStations = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-fields")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "desired_fields = { %s };", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }


    } else if (!strcmp(argv[i], "-WindowSize")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "WindowSize=%s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-NumPasses")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "NumPasses=%s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
    } else if (!strcmp(argv[i], "-MinWeight")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "MinWeight=%s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-MaxAltError")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "MaxAltError=%s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-terrain")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "terrain_file = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-Outside")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "AllowOutsideTerrain = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	

    } else if (!strcmp(argv[i], "-nx")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "nx = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ny")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "ny = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	

    } else if (!strcmp(argv[i], "-dx")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "dx = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-dy")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "dy = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
    } else if (!strcmp(argv[i], "-lat")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "lat_origin = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-lon")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "lon_origin = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
		
    } else if (!strcmp(argv[i], "-name")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "dataset_name = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
		
    } else if (!strcmp(argv[i], "-out")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "output_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-duration")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "duration = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-source")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "input_source = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-sounding")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "sounding_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }


    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	date_time_t start;
	if (sscanf(argv[++i], "%4d %2d %2d %2d %2d %2d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	  start_spec=TRUE;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	date_time_t end;
	if (sscanf(argv[++i], "%4d %2d %2d %2d %2d %2d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	  end_spec=TRUE;
	}
      } else {
	iret = -1;
      }
	
    }
	
    
  } // i

  TR_spec = ((end_spec) && (start_spec));

  if (iret) {
    _usage(prog_name, cerr);
  }

  return iret;
  
}

void Args::_usage(const string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -check_params ] check parameter usage\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "                              ARCHIVE mode only\n"
      << "       [ -mdebug level ] set malloc debug level\n"
      << "       [ -mode ?] ARCHIVE or REALTIME\n"
      << "       [ -params ?] params file path\n"
      << "       [ -print_params ] print parameters with comments\n"
      << "       [ -print_short ] print parameters - short version\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "                                  ARCHIVE mode only\n"
      << "       [ -verbose ] print verbosedebug messages\n"
      << "       [ -interval N ] sets trigger interval to N secs\n"
      << "       [ -nx N ] Sets number of points in X.\n"
      << "       [ -ny N ] Sets number of points in Y.\n"
      << "       [ -WindowSize N] Sets VFR image median filter.\n"
      << "           Tile size is 2*W +1 by 2*w +1 - default is 0.\n"
      << "       [ -NumPasses N ] Passes of VFR filter. Default 0.\n"
      << "       [ -MinWeight ] Necessary Barnes weight => radius. Default 1e-4.\n"
      << "       [ -dx X ] Sets grid spacing in X.\n"
      << "       [ -dy Y ] Sets grid spacing in Y.\n"
      << "       [ -lat X ] Sets latitude of origin.\n"
      << "       [ -lon X ] Sets longitude of origin.\n"
      << "       [ -name X ] Dataset name (written to output MDV file).\n"
      << "       [ -out X ] Output directory name.\n"
      << "       [ -minstations n ] Set the minimum number of stations required.\n"
      << "       [ -duration T ] Time before start time to search database, seconds.\n"
      << "       [ -source S ] Point data source, eg. 63360@couloir.\n"
      << "       [ -terrain f ] Terrain file to subtract from ceiling.\n"
      << "       [ -sounding S ] Directory to search for sounding data.\n"
      << "       [ -latlon ] Use lat/lon rather than flat earth projection.\n"
      << "       [ -fields \"N1, N2, .. Nn\" ]\n"
      << "         Specify grib numbers for fields to put in MDV file.\n"
      << "         Field codes have the following significance.\n\n"
      << "         Code\tSignificance\n"
      << "         33\tU wind *\n"
      << "         34\tV wind *\n"
      << "         11\tTemperature, C *\n"
      << "         17\tDew point temp, C *\n"
      << "         131\tLifted index - requires soundings\n"
      << "         52\tRelative humidity\n"
      << "         118\tWind gust speed\n"
      << "         1\tPressure mb *\n"
      << "         263\tliquid_accum\n"
      << "         59\tprecip_rate\n"
      << "         20\tvisibility *\n"
      << "         153\trvr - Runway visual range *\n"
      << "         154\tCloud height *\n"
      << "         170\tConvergance - calculated from U and V *\n"
      << "         185\tSea level corrected cloud ceiling\n"
      << "         186\tFlight Rules\n"
      << "         5\tTerrain\n"
      << "     A star \"*\" indicates that a field is included by default.\n"
      << "     U and V MUST be specified before convergance to get convergance.\n"
      << endl;
  
  out << "NOTE: for ARCHIVE mode, you must specify the times\n"
      << "      using start and end." << endl;

  out << endl;

  Params::usage(out);

}










