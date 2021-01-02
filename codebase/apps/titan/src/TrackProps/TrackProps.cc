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
// TrackProps.cc
//
// TrackProps object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
///////////////////////////////////////////////////////////////

#include "TrackProps.h"
#include <toolsa/str.h>

// Constructor

TrackProps::TrackProps(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  _name = STRdup("TrackProps");
  ucopyright(_name);

  // get command line args

  _args = new Args(argc, argv, _name);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _name);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  _params = new Params(_args->paramsFilePath,
		       &_args->override,
		       _name,
		       _args->checkParams,
		       _args->printParams,
		       _args->printShort);
  
  if (!_params->OK) {
    fprintf(stderr, "ERROR: %s\n", _name);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }
  if (_params->Done) {
    Done = TRUE;
    return;
  }

  if (!OK) {
    return;
  }

  // input file object

  _input = new InputFile(_name,
			 _params->p.debug,
			 _args->nFiles,
			 _args->filePaths);

  // initialize process registration
  
  PMU_auto_init(_name, _params->p.instance, PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

TrackProps::~TrackProps()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  delete _input;
  delete _params;
  delete _args;

}

#include "TrackFile.h"
using namespace std;

//////////////////////////////////////////////////
// Run

int TrackProps::Run ()
{

  // Print out header

  _printHeader(stdout);
  
  // process all input files

  TrackFile *trackFile = new TrackFile(_name, &_params->p);
  
  char *track_file_path;
  while ((track_file_path = _input->next()) != NULL) {
    trackFile->process(track_file_path);
  }

  delete(trackFile);

  return (0);

}

//////////////////
// _printHeader()
//

void TrackProps::_printHeader(FILE *out)

{

  fprintf(out, "#Program %s\n", _name);

  date_time_t file_time;
  ulocaltime(&file_time);
  fprintf(out, "#File create time: %s\n", utimestr(&file_time));
  
  fprintf(out, "#Min duration (secs): %g\n", _params->p.min_duration);
  
  fprintf(out,
	  "#labels: %s\n",
	  "Count,"
	  "Unixtime,"
	  "Nscans,"
	  "Duration(hr),"
	  "GaussD(hr),"
	  "Gauss_Amean(km2),"
	  "dbzThresh,"
	  "dbzFitMean,"
	  "dbzFitMin,"
	  "dbzFitMax,"
	  "startX(km),"
	  "startY(km),"
	  "meanU(km/h),"
	  "meanV(km/h),"
	  "Speed(km/hr),"
	  "Dirn(degT),"
	  "ellipseRatio,"
	  "ellipseOrient(degT),"
	  );

  fprintf(out, "#\n");

}



