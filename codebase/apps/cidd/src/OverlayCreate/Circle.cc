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
// Circle.cc
//
// Circle class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#include "Circle.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
using namespace std;

//////////////
// Constructor

Circle::Circle(const char *prog_name,
	       const Params &params,
	       double lat,
	       double lon,
	       double radius,
	       int npoints) :
        _params(params)
  
{

  _progName = STRdup(prog_name);
  _lat = lat;
  _lon = lon;
  _radius = radius;
  _nPoints = npoints;
  
  _points = (circle_pt_t *) umalloc (_nPoints * sizeof(circle_pt_t));

}

/////////////
// destructor

Circle::~Circle()

{

  STRfree(_progName);
  ufree(_points);

}

////////////////////////////
// compute()
//
// Compute the circle.
//

void Circle::compute()

{

  double dtheta = 360.0 / _nPoints;

  for (int i = 0; i < _nPoints; i++) {

    double theta = dtheta * i;
    
    PJGLatLonPlusRTheta(_lat, _lon, _radius, theta,
			&_points[i].lat, &_points[i].lon);
  } // i

}

////////////////////////////
// write()
//
// Write the circle points to stdout
//

void Circle::write()

{

  fprintf(stdout, "#Circle lat %g, lon %g, radius %g\n",
	  _lat, _lon, _radius);

  fprintf(stdout, "POLYLINE CIRCLE %d\n", _nPoints + 2);

  for (int i = 0; i < _nPoints; i++) {
    fprintf(stdout, "%10.4f %10.4f\n", _points[i].lat, _points[i].lon);
  }
  fprintf(stdout, "%10.4f %10.4f\n", _points[0].lat, _points[0].lon);
  fprintf(stdout, "%10.4f %10.4f\n", -1000.0, -1000.0);

}

