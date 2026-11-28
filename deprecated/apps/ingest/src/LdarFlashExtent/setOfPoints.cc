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

#include "setOfPoints.hh"
#include <toolsa/umisc.h>
#include<toolsa/pjg_flat.h>
#include <rapformats/GenPt.hh>
#include <cstdio>
#include <rapformats/ltg.h>

//
// Constructor.
//
setOfPoints::setOfPoints(Params *TDRP_Params,
			 double lat,
			 double lon,
			 double alt,
			 double t){

  _params = TDRP_Params;
  _lastLat = lat;   _lastLon = lon;   _lastAlt = alt;   _lastT = t;
  _firstLat = lat;  _firstLon = lon;  _firstAlt = alt;  _firstT = t;
  _numPoints = 1;
  _maxAlt = alt;  _minAlt = alt;  _altVector.push_back(alt);

  
  if (_params->useLatlon){
    //
    // Actually have to do this by setting up header
    //
    Mdvx::field_header_t fhdr;
    memset(&fhdr, 0, sizeof(fhdr));
    fhdr.nx = _params->gridDef.nx;
    fhdr.ny = _params->gridDef.ny;
    fhdr.grid_dx = _params->gridDef.dx;
    fhdr.grid_dy = _params->gridDef.dy;
    fhdr.proj_origin_lat =  _params->gridDef.latOrigin;
    fhdr.proj_origin_lon =  _params->gridDef.lonOrigin;
    fhdr.grid_minx = _params->gridDef.minx;
    fhdr.grid_miny = _params->gridDef.miny;
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    _proj.init( fhdr );
  } else {
    _proj.initFlat(_params->gridDef.latOrigin, _params->gridDef.lonOrigin, 0.0);
    _proj.setGrid(_params->gridDef.nx, _params->gridDef.ny,
		  _params->gridDef.dx, _params->gridDef.dy,
		  _params->gridDef.minx,
		  _params->gridDef.miny);
  }

  return;

}
//
// Method to add a point. Returns 0 if the point
// was added, 1 if the point was not but it is possible that
// another one could be (by looking at the time) and -1 if
// the point was not added and in fact there is no way that
// another point can be added at this time.
//
int setOfPoints::addPoint(double lat,
			  double lon,
			  double alt,
			  double t){

  //
  // If we are out of temporal range, return -1.
  //
  if (t - _lastT > _params->maxAllowedTime){
    return -1;
  }

  //
  // If we are exceeding the max duration, return -1.
  //
  if (t - _firstT > _params->maxDuration){
    return -1;
  }

  //
  // If we are within spatial range, add the point and return 0.
  // Requires that the distances be calculated.
  //
  double r,theta, dist;
  PJGLatLon2RTheta(lat,
		   lon,
		   _lastLat,
		   _lastLon,
		   &r, &theta);
  if (_params->considerVerticalDistance){

    double v_dist = fabs(_lastAlt - alt)/1000.0;
    dist = sqrt(r*r + v_dist*v_dist);
  } else {
    dist = r;
  }
  //
  if (dist > _params->maxAllowedDist){
    //
    // We will not add this point but it is possible that we will
    // add one still.
    //
    return 1;
  }

  //
  // Add the point.
  //
  _lastLat = lat;   _lastLon = lon;   _lastAlt = alt;   _lastT = t;
  if (alt > _maxAlt) _maxAlt = alt;
  if (alt < _minAlt) _minAlt = alt;
  _altVector.push_back(alt);

  int ix, iy;
  if (0== _proj.latlon2xyIndex(lat,lon,ix,iy)){

    bool needToAdd = true;
    for (unsigned k=0; k < _xPoints.size(); k++){
      if ((ix == _xPoints[k]) && (iy == _yPoints[k])){
	needToAdd = false; break;
      }
    }

    if (needToAdd){
      _xPoints.push_back(ix); _yPoints.push_back(iy); 
    }
  }

  _numPoints++;
  //
  return 0;
  //
}



//
// Return the number of points.
//
int setOfPoints::getSize(){
  return _numPoints;
}
//
// Return the duration.
//
double setOfPoints::getDuration(){
  return _lastT - _firstT;
}

//
// Return the number of x,y pairs
//
int setOfPoints::getNumPairs(){
  return (int)_xPoints.size();
}

//
// Return Nth X index
//
int setOfPoints::getNthX(int n){
  return _xPoints[n];
}

//
// Return Nth Y index
//
int setOfPoints::getNthY(int n){
  return _yPoints[n];
}

//
// Destructor. Does nothing.
//
setOfPoints::~setOfPoints(){
  return;
}

