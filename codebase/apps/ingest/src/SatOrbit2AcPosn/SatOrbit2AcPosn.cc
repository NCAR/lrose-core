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
///////////////////////////////////////////////////////////////
// SatOrbit2AcPosn.cc
//
// SatOrbit2AcPosn object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// SatOrbit2AcPosn reads and ascii file containing satellite
// location data, and writes it to SPDB in ac_posn format.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ucopyright.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
#include "SatOrbit2AcPosn.hh"
using namespace std;

// Constructor

SatOrbit2AcPosn::SatOrbit2AcPosn(int argc, char **argv)

{
  
  isOK = true;
  
  // set programe name
  
  _progName = "SatOrbit2AcPosn";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }
  
  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SatOrbit2AcPosn::~SatOrbit2AcPosn()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SatOrbit2AcPosn::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // check if start and end time is set
  
  bool checkTimes = false;
  if (_args.startTime != 0 && _args.endTime != 0) {
    checkTimes = true;
  }

  // create Spdb output object

  DsSpdb spdb;

  // open input file
  
  FILE *in;
  if ((in = fopen(_params.input_file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SatOrbit2AcPosn::Run" << endl;
    cerr << "  Cannot open input file: " << _params.input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in file

  while (!feof(in)) {

    char line[1024];

    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    date_time_t sattime;
    double nadir_lat, nadir_lon;
    double right_lat, right_lon;
    double left_lat, left_lon;

    bool foundAll = false;
    bool foundNadir = false;

    if (sscanf(line, "%d %d %d %d %d %d %lg %lg %lg %lg %lg %lg",
	       &sattime.year, &sattime.month, &sattime.day,
	       &sattime.hour, &sattime.min, &sattime.sec,
	       &nadir_lon, &nadir_lat,
	       &right_lon, &right_lat,
	       &left_lon, &left_lat) == 12) {
      foundNadir = true;
      foundAll = true;
    } else if (sscanf(line, "%d %d %d %d %d %d %lg %lg",
		      &sattime.year, &sattime.month, &sattime.day,
		      &sattime.hour, &sattime.min, &sattime.sec,
		      &nadir_lon, &nadir_lat) == 8) {
      foundNadir = true;
    }

    if (_params.debug) {
      if (foundAll) {
	cerr << "Found all: " << line;
      } else if (foundNadir) {
	cerr << "Found nadir: " << line;
      }
    }

    if (foundNadir) {

      if (_params.apply_bounding_box) {
	if (nadir_lat < _params.bounding_box.min_lat ||
	    nadir_lat > _params.bounding_box.max_lat ||
	    nadir_lon < _params.bounding_box.min_lon ||
	    nadir_lon > _params.bounding_box.max_lon) {
	  continue;
	}
      }

      uconvert_to_utime(&sattime);

      ac_posn_t posn;
      MEM_zero(posn);
      posn.lon = nadir_lon;
      posn.lat = nadir_lat;
      posn.alt = 0;
      STRncopy(posn.callsign, _params.nadir_label, AC_POSN_N_CALLSIGN);
      BE_from_ac_posn(&posn);
      
      spdb.addPutChunk(Spdb::hash4CharsToInt32(_params.nadir_label),
		       sattime.unix_time,
		       sattime.unix_time + 300,
		       sizeof(posn),
		       &posn);

      if (foundAll && _params.store_swath_positions) {

	posn.lon = right_lon;
	posn.lat = right_lat;
	posn.alt = 10;
	STRncopy(posn.callsign, _params.swath_right_label, AC_POSN_N_CALLSIGN);
	BE_from_ac_posn(&posn);
	
	spdb.addPutChunk(Spdb::hash4CharsToInt32(_params.swath_right_label),
			 sattime.unix_time,
			 sattime.unix_time + 300,
			 sizeof(posn),
			 &posn);

	posn.lon = left_lon;
	posn.lat = left_lat;
	posn.alt = 20;
	STRncopy(posn.callsign, _params.swath_left_label, AC_POSN_N_CALLSIGN);
	BE_from_ac_posn(&posn);
	
	spdb.addPutChunk(Spdb::hash4CharsToInt32(_params.swath_left_label),
			 sattime.unix_time,
			 sattime.unix_time + 300,
			 sizeof(posn),
			 &posn);
	
      }

    } // if (foundNadir)

  } // while

  // close the file

  fclose(in);

  // if got data, put it to Spdb

  if (spdb.nPutChunks() > 0) {

    if (spdb.put(_params.output_url,
		 SPDB_AC_POSN_ID,
		 SPDB_AC_POSN_LABEL)) {
      cerr << "ERROR - SatOrbit2AcPosn::Run" << endl;
      cerr << "  Cannot put data to URL: " << _params.output_url << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }

  }
  
  return (0);

}





