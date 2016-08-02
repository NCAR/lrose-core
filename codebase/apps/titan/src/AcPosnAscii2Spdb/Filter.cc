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
/////////////////////////////////////////////////////////////
// Filter.cc
//
// Filter class
//
// Filters ascii ac_posn data into SPDB
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1999
//
/////////////////////////////////////////////////////////////

#include "Filter.hh"
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
using namespace std;

Filter::Filter (const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{
  _isOK = true;
}

Filter::~Filter ()

{
}

int Filter::process_file (const char *input_file_path)

{
  
  PMU_auto_register("Processing file");

  FILE *inFile;

  if ((inFile = fopen(input_file_path, "r")) == NULL) {
    if (_params.debug) {
      cerr << "WARNING - no input file: " << input_file_path << endl;
    }
    return (0);
  }

  cerr << "Processing file: " << input_file_path << endl;

  char line[1024];

  while (!feof(inFile)) {

    if (fgets(line, 1024, inFile) != NULL) {

      unsigned ac_time;
      double lat, lon, alt;
      char callsign[128];
      
      // interpret line
      
      if (sscanf(line, "%d %lg %lg %lg %s",
		 &ac_time, &lat, &lon, &alt, callsign) != 5) {
	if (_params.debug) {
	  cerr << "WARNING - cannot decode line: " << endl;
	  cerr << "  " << line;
	}
      }

      // correct if alt is in m instead of km

      if (alt > 100.0) {
	alt /= 1000.0;
      }
      
      // load struct

      ac_posn_t posn;
      posn.lat = lat;
      posn.lon = lon;
      posn.alt = alt;
      STRncopy(posn.callsign, callsign, AC_POSN_N_CALLSIGN);

      // set to bigend format
      
      BE_from_ac_posn(&posn);
      
      // compute chunk type
      
      si32 chunk_type = Spdb::hash4CharsToInt32(posn.callsign);

      // store in data base

      Spdb spdb;
      if (spdb.put(_params.output_data_dir,
                   AC_POSN_PROD_CODE,
                   AC_POSN_PROD_LABEL,
                   chunk_type,
                   ac_time,
                   ac_time + 120,
                   sizeof(ac_posn_t),
                   &posn)) {
	cerr << "WARNING - " << _progName << endl;
	cerr << "Could not store SPDB data in dir: "
	     << _params.output_data_dir << endl;
        cerr << spdb.getErrStr() << endl;
      }

    } // if (fgets(line, 1024, inFile) != NULL) 
      
  } // while
  
  fclose(inFile);

  return (0);

}


