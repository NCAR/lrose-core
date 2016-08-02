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
// MapCircles.cc
//
// MapCircles object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2002
//
///////////////////////////////////////////////////////////////////////
//
// MapCircles creates a map file, with circles of a given radius
// around a set of points.
// Map file data is written to stdout.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/ucopyright.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/DateTime.hh>

#include "MapCircles.hh"
using namespace std;

// Constructor

MapCircles::MapCircles(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MapCircles";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
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

MapCircles::~MapCircles()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MapCircles::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // header

  fprintf(stdout, "#\n");
  fprintf(stdout, "# %s\n", _params.map_file_header);
  fprintf(stdout, "#\n");
  fprintf(stdout, "# Created %s\n", DateTime::str().c_str());
  fprintf(stdout, "#\n");

  // arcs

  for (int ii = 0; ii < _params.arcs_n; ii++) {

    const Params::arc_t &arc = _params._arcs[ii];
    double startAz = arc.start_az_deg;
    double endAz = arc.end_az_deg;
    if (startAz > endAz) {
      startAz -= 360.0;
    }
    double azRange = endAz - startAz;
    int nSeg = arc.n_segments;
    double deltaAz = azRange / nSeg;
    double radius = arc.radius_km;

    fprintf(stdout, "POLYLINE %s %d\n", arc.label, nSeg + 1);
    
    for (int jj = 0; jj < nSeg + 1; jj++) {

      double az = startAz + jj * deltaAz;
      double lat, lon;
      
      PJGLatLonPlusRTheta(arc.center_lat, arc.center_lon,
			  radius, az, &lat, &lon);
      
      fprintf(stdout,"%12.6f %12.6f\n", lat, lon);

    } // jj

    fprintf(stdout, "\n");

  } //  ii

  // azimuth lines

  for (int ii = 0; ii < _params.az_lines_n; ii++) {

    const Params::az_line_t &az_line = _params._az_lines[ii];

    double startAz = az_line.start_az_deg;
    double deltaAz = az_line.delta_az_deg;
    int nLines = az_line.n_lines;

    double minRadius = az_line.min_radius_km;
    double maxRadius = az_line.max_radius_km;
    
    fprintf(stdout, "POLYLINE %s %d\n", az_line.label, nLines * 3);
    
    for (int jj = 0; jj < nLines; jj++) {
      
      double az = startAz + jj * deltaAz;
      double lat, lon;
      
      // start of line

      PJGLatLonPlusRTheta(az_line.center_lat, az_line.center_lon,
			  minRadius, az, &lat, &lon);

      fprintf(stdout, "%12.6f %12.6f\n", lat, lon);

      // start of line

      PJGLatLonPlusRTheta(az_line.center_lat, az_line.center_lon,
			  maxRadius, az, &lat, &lon);
      
      fprintf(stdout, "%12.6f %12.6f\n", lat, lon);

      // penup

      fprintf(stdout, "-1000 -1000\n");

    } // jj

    fprintf(stdout, "\n");

  } //  ii

  return (0);

}

