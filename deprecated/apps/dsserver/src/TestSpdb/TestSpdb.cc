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
// TestSpdb.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <spdb/Spdb.hh>
#include <spdb/spdb_products.h>
#include "TestSpdb.hh"
using namespace std;
#include "Args.hh"
#include "Params.hh"
#include "TestSpdb.hh"
using namespace std;

// Constructor

TestSpdb::TestSpdb(int argc, char **argv)

{

  OK = TRUE;

  // set programe name
  
  _progName = "TestSpdb";

  // parse command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // initialize Procmap module

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

TestSpdb::~TestSpdb()

{

  // unregister with Procmap

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TestSpdb::Run()
{

  // set up SPDB output

  Spdb spdb;
  if (_params.load_mode == Params::LOAD_OVERWRITE) {
    spdb.setPutMode(Spdb::putModeOver);
  } else if (_params.load_mode == Params::LOAD_ADD) {
    spdb.setPutMode(Spdb::putModeAdd);
  } else {
    spdb.setPutMode(Spdb::putModeOnce);
  }

  spdb.clearUrls();
  for (int i = 0; i < _params.destination_urls_n; i++) {
    spdb.addUrl(_params._destination_urls[i]);
  }

  if (_params.no_threads) {
    spdb.setPutThreadingOff();
  }
  
  // open the data file

  FILE *in;
  if ((in = fopen(_params.source_file_path, "r")) == NULL) {
    perror(_params.source_file_path);
    return (-1);
  }

  char line[1024];
  char str[1024];
  date_time_t dt;

  int count = 0;
  
  while (fgets(line, 1024, in) != NULL) {
    
    if (count == 0 && _params.chunks_per_put > 1) {
      spdb.clearChunks();
    }
    
    if (line[0] == '#') {
      continue;
    }
    
    if (_params.date_time_format == Params::UNIX_TIME) {
      if (sscanf(line, "%ld%s", &dt.unix_time, str) == 2) {
	uconvert_from_utime(&dt);
      } else {
	fprintf(stderr, "Bad line: %s", line);
	continue;
      }
    } else {
      if (sscanf(line, "%d%d%d%d%d%d%s",
		 &dt.year, &dt.month, &dt.day,
		 &dt.hour, &dt.min, &dt.sec, str) == 7) {
	uconvert_to_utime(&dt);
      } else {
	fprintf(stderr, "Bad line: %s", line);
	continue;
      }
    }

    if (_params.chunks_per_put > 1) {
      spdb.addChunk(count,
		    dt.unix_time,
		    dt.unix_time + _params.product_valid_period,
		    strlen(str) + 1,
		    str);
    }

    count++;

    if (count == _params.chunks_per_put) {
      
      if (_params.chunks_per_put == 1) {
	
	if (_params.destination_urls_n == 1) {

	  if (_params.debug) {
	    cerr << "Writing single chunk to single URL" << endl;
	  }

	  if (spdb.put(_params._destination_urls[0],
		       SPDB_ASCII_ID,
		       SPDB_ASCII_LABEL,
		       count,
		       dt.unix_time,
		       dt.unix_time + _params.product_valid_period,
		       strlen(str) + 1,
		       str)) {
	    cerr << "ERROR - single chunk to single URL" << endl;
	  }
	  
	} else {
	  
	  if (_params.debug) {
	    cerr << "Writing single chunks to multi URLs" << endl;
	  }

	  if (spdb.put(SPDB_ASCII_ID,
		       SPDB_ASCII_LABEL,
		       count,
		       dt.unix_time,
		       dt.unix_time + _params.product_valid_period,
		       strlen(str) + 1,
		       str)) {
	    cerr << "ERROR - single chunk to multi URLs" << endl;
	  }
	  
	} // if (_params.destination_urls_n == 1) 

      } else {

	if (_params.destination_urls_n == 1) {
	  
	  if (_params.debug) {
	    cerr << "Writing multi chunks to single URL" << endl;
	  }

	  if (spdb.put(_params._destination_urls[0],
		       SPDB_ASCII_ID,
		       SPDB_ASCII_LABEL)) {
	    cerr << "ERROR - multi chunks to single URL" << endl;
	  }

	} else {
	  
	  if (_params.debug) {
	    cerr << "Writing multi chunks to multi URLs" << endl;
	  }

	  if (spdb.put(SPDB_ASCII_ID,
		       SPDB_ASCII_LABEL)) {
	    cerr << "ERROR - multi chunks to multi URLs" << endl;
	  }
	  
	} // if (_params.destination_urls_n == 1) 

      } // if (_params.chunks_per_put == 1) 

    } // if (count == _params.chunks_per_put) 

    if (count == _params.chunks_per_put) {
      count = 0;
    }

  } // while

  fclose(in);

  // give the threads time to complete

  if (!_params.no_threads) {
    sleep(10);
  }

  return (0);
  
}


