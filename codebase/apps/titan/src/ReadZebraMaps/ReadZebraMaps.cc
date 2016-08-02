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
// ReadZebraMaps.cc
//
// ReadZebraMaps object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////

#include "ReadZebraMaps.hh"
#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>

using namespace std;

// Constructor

ReadZebraMaps::ReadZebraMaps(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = strdup("ReadZebraMaps");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  return;

}

// destructor

ReadZebraMaps::~ReadZebraMaps()

{

  // free up

  delete(_params);
  delete(_args);
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int ReadZebraMaps::Run()
{

  // open input file
  
  FILE *in;
  if ((in = fopen(_params->input_file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ReadZebraMaps" << endl;
    cerr << "  Cannot open file: " << _params->input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  int polyNum = 0;
  
  while (!feof(in)) {

    // read the number of points

    int npts;
    if (fscanf(in, "%d", &npts) != 1) {
      if (feof(in)) {
	fclose(in);
	return 0;
      } else {
	cerr << "ERROR - ReadZebraMaps" << endl;
	cerr << "  Cannot decode file: " << _params->input_file_path << endl;
	cerr << "  Trying to read npts" << endl;
	fclose(in);
	return -1;
      }
    }
    npts /= 2;

    // read in bounding box

    double minLat, maxLat, minLon, maxLon;
    if (fscanf(in, "%lg %lg %lg %lg",
	       &minLat, &maxLat, &minLon, &maxLon) != 4) {
      cerr << "ERROR - ReadZebraMaps" << endl;
      cerr << "  Cannot decode file: " << _params->input_file_path << endl;
      cerr << "  Trying to read bounding box" << endl;
      fclose(in);
      return -1;
    }

    if (_params->output == Params::RAP_MAP) {
      cout << "POLYLINE " << polyNum << " " << npts << endl;
    }

    // scan in lat/lon points
    
    for (int i = 0; i < npts; i++) {

      double lat, lon;
      
      if (fscanf(in, "%lg %lg", &lat, &lon) != 2) {
	cerr << "ERROR - ReadZebraMaps" << endl;
	cerr << "  Cannot decode file: " << _params->input_file_path << endl;
	cerr << "  Trying to read lat/lon point" << endl;
	fclose(in);
	return -1;
      }

      if (_params->output == Params::RAP_MAP) {
	cout << lat << " " << lon << endl;
      } else if (_params->output == Params::SIGMET_MAP) {
	cout << "POINT " << lon << " " << lat << endl;
      }
      
    }

    if (_params->output == Params::RAP_MAP) {
      cout << "-1000.0 -1000.0" << endl << endl;
    } else if (_params->output == Params::SIGMET_MAP) {
      cout << "GAP" << endl;
    }

    polyNum++;
	 
  } // while (!feof ...

  fclose(in);
  return 0;

}
