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
// ModelGrid.cc
//
// This class is responsible for computations concerning the 
// model grid localtions.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////

#include "ModelGrid.hh"
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <math.h>
using namespace std;

///////////////
// Constructor

ModelGrid::ModelGrid (char *prog_name, Params *params,
		      int model_nlat, int model_nlon,
		      fl32 **model_lat, fl32 **model_lon)

{

  _progName = STRdup(prog_name);
  _params = params;
  _nLat = model_nlat;
  _nLon = model_nlon;

  _lat = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _lon = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  memcpy(*_lat, *model_lat, _nLat * _nLon * sizeof(fl32));
  memcpy(*_lon, *model_lon, _nLat * _nLon * sizeof(fl32));

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Model SW corner: %g, %g\n",
	    _lat[0][0], _lon[0][0]);
    fprintf(stderr, "Model NW corner: %g, %g\n",
	    _lat[_nLat-1][0], _lon[_nLat-1][0]);
    fprintf(stderr, "Model SE corner: %g, %g\n",
	    _lat[0][_nLon-1], _lon[0][_nLon-1]);
    fprintf(stderr, "Model NE corner: %g, %g\n",
	    _lat[_nLat-1][_nLon-1],
	    _lon[_nLat-1][_nLon-1]);
  }

  _minLat = 90.0;
  for (int i = 0; i < _nLon; i++) {
    _minLat = MIN(_minLat, _lat[0][i]);
  }

  _maxLat = -90.0;
  for (int i = 0; i < _nLon; i++) {
    _maxLat = MAX(_maxLat, _lat[_nLat-1][i]);
  }

  _minLon = 180.0;
  for (int i = 0; i < _nLat; i++) {
    _minLon = MIN(_minLon, _lon[i][0]);
  }

  _maxLon = -180.0;
  for (int i = 0; i < _nLat; i++) {
    _maxLon = MAX(_maxLon, _lon[i][_nLon-1]);
  }

  _meanDLat = (_maxLat - _minLat) / (_nLat - 1.0);
  _meanDLon = (_maxLon - _minLon) / (_nLon - 1.0);


  if (_params->debug >= Params::DEBUG_NORM) {
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

ModelGrid::~ModelGrid()

{
  ufree2((void **) _lat);
  ufree2((void **) _lon);
}

////////////////
// findModelLoc
//
// Find the model location for a given lat/lon
//
// returns 0 on success, -1 on failure

#define MAX_MOVES 128

int ModelGrid::findModelLoc(double lat, double lon)

{

  int nMoves = 0;
  int move_lat = TRUE;
  
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

    // fprintf(stderr, "iLat, iLon: %d, %d\n", iLat, iLon);
    // fprintf(stderr, "minlat, maxlat: %g, %g\n", minlat, maxlat);
    // fprintf(stderr, "minlon, maxlon: %g, %g\n", minlon, maxlon);
    
    // test for success

    if (lat >= minlat && lat <= maxlat &&
	lon >= minlon && lon <= maxlon) {

      // set the grid indices

      latIndex = iLat;
      lonIndex = iLon;

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

      move_lat = FALSE;

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
      
      move_lat = TRUE;

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
