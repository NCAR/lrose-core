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
//////////////////////////////////////////////////////////
// WRFGrid.cc
//
// This class is responsible for computations concerning the 
// model grid localtions.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <math.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>
#include "Era5Data.hh"
#include "WRFGrid.hh"

using namespace std;

///////////////
// Constructor


WRFGrid::WRFGrid (const string &prog_name, bool debug, Era5Data &inData):
  _progName(prog_name), _debug(debug)
{
  inData.initProjection(proj);

  _minx = inData.getGridMinX();
  _dx = inData.getGridDx();
  _miny = inData.getGridMinY();
  _dy = inData.getGridDy();
  _midLon = 0.0;

  _nLat = inData.getNLat();
  _ny = inData.getNLat();

  _nLon = inData.getNLon();
  _nx = inData.getNLon();

  _lat = NULL;
  _lon = NULL;
  _lat = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _lon = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  memcpy(*_lat, *inData.getLat(), _nLat * _nLon * sizeof(fl32));
  memcpy(*_lon, *inData.getLon(), _nLat * _nLon * sizeof(fl32));
}

///////////////
// Constructor

WRFGrid::WRFGrid (const string &prog_name, bool debug,
		  int model_nlat, int model_nlon,
		  fl32 **model_lat, fl32 **model_lon) :
  _progName(prog_name), _debug(debug)

{

  _nLat = model_nlat;
  _nLon = model_nlon;
  _ny = model_nlat;
  _nx = model_nlon;

  _lat = NULL;
  _lon = NULL;
  _lat = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _lon = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  memcpy(*_lat, *model_lat, _nLat * _nLon * sizeof(fl32));
  memcpy(*_lon, *model_lon, _nLat * _nLon * sizeof(fl32));

  if (_debug) {
    fprintf(stderr, "Model SW corner: %g, %g\n",
	    _lat[0][0], _lon[0][0]);
    fprintf(stderr, "Model NW corner: %g, %g\n",
	    _lat[_nLat-1][0], _lon[_nLat-1][0]);
    fprintf(stderr, "Model NE corner: %g, %g\n",
	    _lat[_nLat-1][_nLon-1],
	    _lon[_nLat-1][_nLon-1]);
    fprintf(stderr, "Model SE corner: %g, %g\n",
	    _lat[0][_nLon-1], _lon[0][_nLon-1]);
  }

  // condition the longitudes to be in the same hemisphere
  // Search for Lat - lon ranges.

  _minLat = 90.0;
  _maxLat = -90.0;
  _minLon = 360.0;
  _maxLon = -360.0;
  _midLon = (_minLon + _maxLon) /2.0;
  for (int ilat = 0; ilat < _nLat; ilat++) {
    for (int ilon = 0; ilon < _nLon; ilon++) {
      _lon[ilat][ilon] = _conditionLon(_lon[ilat][ilon]);

      _minLat = MIN(_minLat, _lat[ilat][ilon]);
      _maxLat = MAX(_maxLat, _lat[ilat][ilon]);
      _minLon = MIN(_minLon, _lon[ilat][ilon]);
      _maxLon = MAX(_maxLon, _lon[ilat][ilon]);
    }
  }

  _meanDLon = (_maxLon - _minLon) / (_nLon - 1.0);
  _meanDLat = (_maxLat - _minLat) / (_nLat - 1.0);


  if (_debug) {
    fprintf(stderr, "model nlat, nlon: %d, %d\n",
	    _nLat, _nLon);
    fprintf(stderr, "model minlat, maxlat: %g, %g\n",
	    _minLat, _maxLat);
    fprintf(stderr, "model minlon, maxlon: %g, %g\n",
	    _minLon, _maxLon);
    fprintf(stderr, "model meanDLat, meanDLon: %g, %g\n",
	    _meanDLat, _meanDLon);
  }

}

//////////////
// destructor

WRFGrid::~WRFGrid()

{
  if (_lat) {
    ufree2((void **) _lat);
  }
  if (_lon) {
    ufree2((void **) _lon);
  }
}

///////////////////////////////////////////////
// getGridIndices
//
// Find the model location for a given lat/lon
// Set inverse distance weights.
// Is functional equivilent to findModelLoc, but solves it analitically.
// Uses great circle distances, rather than sperical-polar distance.
// Is valid for grids that span the poles (Polar Stereographic grids).
// returns true on success, false on failure

bool WRFGrid::getGridIndices(double lat, double lon)
{
  double theta;
  double x,y;
  double d1,d2,d3,d4,dtot;
  int ilon,ilat;  // Grid indicies.

  // condition longitude
  lon = _conditionLon(lon);

  proj.latlon2xy(lat,lon,x,y);
  ilon = (int) floor((x - _minx)/_dx);  // take floor to get SW corner index.
  ilat = (int) floor((y - _miny)/_dy);


  // Check to see if point is internal to grid.
  if (ilon < 0 || ilon >= _nx-1 || ilat < 0 || ilat >= _ny-1) {
      latIndex = 0;
      lonIndex = 0;
      return false;
  } else {
      // fprintf(stderr,"Lat,lon: %8g, %8g  GY,GX:  %5d,%5d\n",lat,lon,ilon,ilat);
      latIndex = ilat;
      lonIndex = ilon;
  }

  // Compute (inverse) distance to SW corner.
  PJGLatLon2RTheta(lat,lon,_lat[ilat][ilon],_lon[ilat][ilon],&d1,&theta);
  if(d1 == 0.0) d1 = 1.0e-20;
  d1 = 1.0/d1;

  // Compute (inverse) distance to NW corner.
  PJGLatLon2RTheta(lat,lon,_lat[ilat+1][ilon],_lon[ilat+1][ilon],&d2,&theta);
  if(d2 == 0.0) d2 = 1.0e-20;
  d2 = 1.0/d2;
  
  // Compute (inverse) distance to NE corner.
  PJGLatLon2RTheta(lat,lon,_lat[ilat+1][ilon+1],_lon[ilat+1][ilon+1],&d3,&theta);
  if(d3 == 0.0) d3 = 1.0e-20;
  d3 = 1.0/d3;
  
  // Compute (inverse) distance to SE corner.
  PJGLatLon2RTheta(lat,lon,_lat[ilat][ilon+1],_lon[ilat][ilon+1],&d4,&theta);
  if(d4 == 0.0) d4 = 1.0e-20;
  d4 = 1.0/d4;

  dtot = d1 + d2 + d3 + d4; // Sum of inverses.

  wtSW = d1 / dtot;
  wtNW = d2 / dtot;
  wtNE = d3 / dtot;
  wtSE = d4 / dtot;

  return true;
}

///////////////////////////////////////////////
// findModelLoc
//
// Find the model location for a given lat/lon
// Warning: Not valid near the poles or for
// WRF grids on Polar Stereographic grids.
//
// returns 0 on success, -1 on failure

#define MAX_MOVES 128

int WRFGrid::findModelLoc(double lat, double lon)

{

  int nMoves = 0;
  bool move_lat = true;

  // condition longitude

  lon = _conditionLon(lon);

  // check globally

  if (lat < _minLat || lat > _maxLat ||
      lon < _minLon || lon > _maxLon) {
    return (-1);
  }

  // computing starting cell location

  int iLat = (int) ((lat - _minLat) / _meanDLat);
  iLat = MAX(iLat, 0);
  iLat = MIN(iLat, _nLat-2);

  int iLon = (int) ((lon - _minLon) / _meanDLon);
  iLon = MAX(iLon, 0);
  iLon = MIN(iLon, _nLon-2);

  int nStuck = 0;

  while (nMoves < MAX_MOVES) {

    double sw_lat = _lat[iLat][iLon];
    double sw_lon = _lon[iLat][iLon];

    double nw_lat = _lat[iLat+1][iLon];
    double nw_lon = _lon[iLat+1][iLon];

    double ne_lat = _lat[iLat+1][iLon+1];
    double ne_lon = _lon[iLat+1][iLon+1];

    double se_lat = _lat[iLat][iLon+1];
    double se_lon = _lon[iLat][iLon+1];

    // get bounding box for this cell

    double minlat = MIN(sw_lat, se_lat);
    double maxlat = MAX(nw_lat, ne_lat);
    double minlon = MIN(sw_lon, nw_lon);
    double maxlon = MAX(ne_lon, se_lon);

    //fprintf(stderr, "iLat, iLon: %d, %d\n", iLat, iLon);
    //fprintf(stderr, "minlat, maxlat: %g, %g\n", minlat, maxlat);
    //fprintf(stderr, "minlon, maxlon: %g, %g\n", minlon, maxlon);
    
    // test for success

    if (lat >= minlat && lat <= maxlat &&
	lon >= minlon && lon <= maxlon) {

      // set the grid indices

      latIndex = iLat;
      lonIndex = iLon;
      //fprintf(stderr,"iLat, iLon, nMoves: %d, %d, %d\n",iLat,iLon,nMoves);

      // compute the interpolation factors

      double dlat, dlon, dist;

      dlat = lat - sw_lat;
      dlon = lon - sw_lon;
      dist = sqrt(dlat * dlat + dlon * dlon);
      if (dist == 0.0) {
	dist = 1.0e-20;
      }
      double sw_inv = 1.0 / dist;

      dlat = lat - nw_lat;
      dlon = lon - nw_lon;
      dist = sqrt(dlat * dlat + dlon * dlon);
      if (dist == 0.0) {
	dist = 1.0e-20;
      }
      double nw_inv = 1.0 / dist;

      dlat = lat - ne_lat;
      dlon = lon - ne_lon;
      dist = sqrt(dlat * dlat + dlon * dlon);
      if (dist == 0.0) {
	dist = 1.0e-20;
      }
      double ne_inv = 1.0 / dist;

      dlat = lat - se_lat;
      dlon = lon - se_lon;
      dist = sqrt(dlat * dlat + dlon * dlon);
      if (dist == 0.0) {
	dist = 1.0e-20;
      }
      double se_inv = 1.0 / dist;
      
      double sum_inv = sw_inv + nw_inv + ne_inv + se_inv;

      wtSW = sw_inv / sum_inv;
      wtNW = nw_inv / sum_inv;
      wtNE = ne_inv / sum_inv;
      wtSE = se_inv / sum_inv;

      return (0);

    }

    // make a move

    if (move_lat) {

      move_lat = false;

      if (lat > maxlat) {
	if (iLat < _nLat-2) {
	  iLat++;
	  nMoves++;
	  nStuck = 0;
	  // fprintf(stderr, "***** moving up\n");
	  continue;
	} else {
	  nStuck++;
	  if (nStuck > 2) {
	    // fprintf(stderr, "#### Stuck at top\n");
	    return (-1);
	  }
	}
      } else if (lat < minlat) {
	if (iLat > 0) {
	  iLat--;
	  nMoves++;
	  nStuck = 0;
	  // fprintf(stderr, "***** moving down\n");
	  continue;
	} else {
	  nStuck++;
	  if (nStuck > 2) {
	    // fprintf(stderr, "#### Stuck at bottom\n");
	    return (-1);
	  }
	}
      }

    } else { // if (move_lat)
      
      move_lat = true;

      if (lon > maxlon) {
	if (iLon < _nLon-2) {
	  iLon++;
	  nMoves++;
	  nStuck = 0;
	  // fprintf(stderr, "***** moving right\n");
	  continue;
	} else {
	  nStuck++;
	  if (nStuck > 2) {
	    // fprintf(stderr, "#### Stuck at right side\n");
	    return (-1);
	  }
	}
      } else if (lon < minlon) {
	if (iLon > 0) {
	  iLon--;
	  nMoves++;
	  nStuck = 0;
	  // fprintf(stderr, "***** moving left\n");
	  continue;
	} else {
	  nStuck++;
	  if (nStuck > 2) {
	    // fprintf(stderr, "#### Stuck at left side\n");
	    return (-1);
	  }
	}
      }
      
    } // if (move_lat)
    
  } // while

  return (0);

}

///////////////////////////////////////////////
//
// Set for non-interpolation at the given point
//
// returns 0 on success, -1 on failure

void WRFGrid::setNonInterp(int ilat, int ilon)
{
  // set the grid indices

  latIndex = ilat;
  lonIndex = ilon;

  wtSW = 1.0;
  wtNW = 0.0;
  wtNE = 0.0;
  wtSE = 0.0;
}

///////////////////////////////////////////////////////////////////
// Condition longitude to be in same hemisphere as mid lon

double WRFGrid::_conditionLon(double lon)

{
  
  double diff = _midLon - lon;
  if (fabs(diff) > 180.0) {
    if (diff > 0) {
      return lon + 360.0;
    } else {
      return lon - 360.0;
    }
  }
  return lon;
  
}
