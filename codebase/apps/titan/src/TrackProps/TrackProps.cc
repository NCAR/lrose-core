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

#include "TrackProps.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>

// Constructor

TrackProps::TrackProps(int argc, char **argv)

{

  OK = TRUE;
  _input = NULL;
  
  // set programe name
  
  _progName = "TrackProps";
  ucopyright(_progName.c_str());

  // parse args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = false;
    return;
  }

  if (!OK) {
    return;
  }

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  // input file object

  _input = new InputFile(_progName.c_str(),
			 _params.debug,
			 _args.filePaths);

}

// destructor

TrackProps::~TrackProps()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  if (_input) {
    delete _input;
  }

}

#include "TrackFile.hh"
using namespace std;

//////////////////////////////////////////////////
// Run

int TrackProps::Run ()
{

  // Print out header

  _printHeader(stdout);
  
  // process all input files

  TrackFile *trackFile = new TrackFile(_progName.c_str(), _params);
  
  while (true) {
    string track_file_path = _input->next();
    if (track_file_path.size() < 1) {
      break;
    }
    trackFile->process(track_file_path.c_str());
  }

  delete(trackFile);

  return (0);

}

//////////////////
// _printHeader()
//

void TrackProps::_printHeader(FILE *out)

{

  fprintf(out, "#Program %s\n", _progName.c_str());

  date_time_t file_time;
  ulocaltime(&file_time);
  fprintf(out, "#File create time: %s\n", utimestr(&file_time));
  
  fprintf(out, "#Min duration (secs): %g\n", _params.min_duration);
  
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



