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
////////////////////////////////////////////////////////////////////////
// MapGrid.cc
//
// MapGrid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2002
//
///////////////////////////////////////////////////////////////////////
//
// MapGrid creates a map file containing an evenly spaced grid
// in latitude and longitude.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/ucopyright.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/DateTime.hh>

#include "MapGrid.hh"
using namespace std;

// Constructor

MapGrid::MapGrid(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MapGrid";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

MapGrid::~MapGrid()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MapGrid::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // header

  fprintf(stdout, "#\n");
  fprintf(stdout, "# %s\n", _params.map_file_header);
  fprintf(stdout, "#\n");
  fprintf(stdout, "# Created %s\n", DateTime::str().c_str());
  fprintf(stdout, "# min_lat, max_lat, delta_lat: %g, %g, %g\n",
	  _params.min_lat, _params.max_lat, _params.delta_lat);
  fprintf(stdout, "# min_lon, max_lon, delta_lon: %g, %g, %g\n",
	  _params.min_lon, _params.max_lon, _params.delta_lon);
  fprintf(stdout, "#\n");

  // lines of latitude

  double lat = _params.min_lat;
  double quarterLon = (_params.max_lon - _params.min_lon) / 4.0;
  while (lat <= _params.max_lat) {
    fprintf(stdout, "POLYLINE %g_deg_lat 5\n", lat);
    fprintf(stdout, "%g %g\n", lat, _params.min_lon);
    fprintf(stdout, "%g %g\n", lat, _params.min_lon + quarterLon);
    fprintf(stdout, "%g %g\n", lat, _params.min_lon + quarterLon * 2);
    fprintf(stdout, "%g %g\n", lat, _params.min_lon + quarterLon * 3);
    fprintf(stdout, "%g %g\n", lat, _params.max_lon);
    fprintf(stdout, "\n");
    lat += _params.delta_lat;
  }
  
  // lines of longitude

  double lon = _params.min_lon;
  while (lon <= _params.max_lon) {
    fprintf(stdout, "POLYLINE %g_deg_lon 2\n", lon);
    fprintf(stdout, "%g %g\n", _params.min_lat, lon);
    fprintf(stdout, "%g %g\n", _params.max_lat, lon);
    fprintf(stdout, "\n");
    lon += _params.delta_lon;
  }
  
  return (0);

}

