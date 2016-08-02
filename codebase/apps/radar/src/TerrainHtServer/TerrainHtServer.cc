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
// IwrfTerrainHtServer.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// TerrainHtServer reads requests from a client, providing a
// lat/lon position. It returns the terrain height, and whether
// the location is water or not.
// 
////////////////////////////////////////////////////////////////

#include "TerrainHtServer.hh"
#include "ServerThread.hh"
#include "SquareDegree.hh"

#include <algorithm>
#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>

using namespace std;

// Constructor

TerrainHtServer::TerrainHtServer(int argc, char **argv)
  
{

  isOK = true;
  _serverThread = NULL;
  _tiles = NULL;
  _latestLat = -9999.0;
  _latestLon = -9999.0;

  // set programe name
  
  _progName = "TerrainHtServer";

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }
  
  // init process mapper registration
  
  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

TerrainHtServer::~TerrainHtServer()

{

  PMU_auto_unregister();

  if (_serverThread) {
    delete _serverThread;
  }

  // free up tiles
  
  if (_tiles != NULL) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        delete _tiles[ilat][ilon];
      }
    }
    ufree2((void **) _tiles);
  }
  
}

//////////////////////////////////////////////////
// Run

int TerrainHtServer::Run ()
{

  // create the tiles

  PMU_auto_register("Creating tiles");
  
  _tiles = (SquareDegree ***) umalloc2(nLat, nLon, sizeof(SquareDegree *));
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double centerLat = ilat + 0.5 - 90.0;
      double centerLon = ilon + 0.5 - 180.0;
      _tiles[ilat][ilon] = new SquareDegree(_params, centerLat, centerLon);
    }
  }
  
  // create server thread, and set server going

  _serverThread = new ServerThread(this, _params);
  _serverThread->signalRunToStart();

  // loop

  while (true) {

    // create cache around latest location

    PMU_auto_register("Updating cache");
    _updateCache();

    // free up memory from unused tiles

    PMU_auto_register("Freeing tile memory");
    _freeTileMemory();

    // sleep a bit

    PMU_auto_register("Zzzzz...");
    umsleep(2500);

  } // while
  
  return 0;

}

//////////////////////////////////////////////////////////
// get terrain ht and water flag for a point
// returns 0 on success, -1 on failure
// sets terrainHtM and isWater args

int TerrainHtServer::getHt(double lat, double lon,
                           double &terrainHtM, bool &isWater)


{

  // if we do not need to check adjacent cells, we just use the central point

  if (_params.check_adjacent_grid_cells == false) {
    return _getHt(lat, lon, terrainHtM, isWater);
  }
  
  double marginKm = _params.search_margin_km;

  double yMarginDeg = marginKm / KM_PER_DEG_AT_EQ;
  double xMarginDeg = yMarginDeg / cos(lat * DEG_TO_RAD);
  
  int yMarginCells = (int) (yMarginDeg / SquareDegree::GridRes + 0.5);
  if (yMarginCells < 1) {
    yMarginCells = 1;
  }

  int xMarginCells = (int) (xMarginDeg / SquareDegree::GridRes + 0.5);
  if (xMarginCells < 1) {
    xMarginCells = 1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "lat, lon, marginKm, yMarginDeg, xMarginDeg, yMarginCells, xMarginCells: "
         << lat << ", " << lon << ", "
         << marginKm << ", "
         << yMarginDeg << ", "
         << xMarginDeg << ", "
         << yMarginCells << ", "
         << xMarginCells << endl;
  }

  double found = false;
  double maxHt = -9999;
  bool gotWater = false;
  for (int ilon = -xMarginCells; ilon <= xMarginCells; ilon++) {
    double searchLon = lon + ilon * SquareDegree::GridRes;
    for (int ilat = -yMarginCells; ilat <= yMarginCells; ilat++) {
      double searchLat = lat + ilat * SquareDegree::GridRes;
      double ht = 0.0;
      bool water = false;
      if (_getHt(searchLat, searchLon, ht, water) == 0) {
        found = true;
        if (water) {
          gotWater = true;
        }
        if (ht > maxHt) {
          maxHt = ht;
        }
      }
    }
  }

  if (found) {
    terrainHtM = maxHt;
    isWater = gotWater;
    return 0;
  } else {
    return -1;
  }

}

//////////////////////////////////////////////////////////
// get terrain ht and water flag for a point
// returns 0 on success, -1 on failure
// sets terrainHtM and isWater args

int TerrainHtServer::_getHt(double lat, double lon,
                            double &terrainHtM, bool &isWater)
  

{

  // condition the longitude and latitude

  if (lon > 180.0) {
    lon -= 360.0;
  }

  // compute tile indices

  int ilat = (int) (lat - -90.0);
  int ilon = (int) (lon - -180.0);

  if (ilat < 0) ilat = 0;
  if (ilat > nLat - 1) ilat = nLat - 1;
  if (ilon < 0) ilon = 0;
  if (ilon > nLon - 1) ilon = nLon - 1;

  if (_tiles[ilat][ilon]->getHt(lat, lon, terrainHtM, isWater)) {
    cerr << "ERROR - TerrainHtServer::getHt()" << endl;
    cerr << "  Cannot get height for lat, lon: " << lat << ", " << lon << endl;
    cerr << "  Tile indices: ilat, ilon: " << ilat << ", " << ilon << endl;
    return -1;
  }
  
  // save location of latest request

  _latestLat = lat;
  _latestLon = lon;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Got ht request: lat, lon: " << lat << ", " << lon << endl;
    cerr << "Got ht request: htM, isWater: " << terrainHtM << ", " << isWater << endl;
  }

  return 0;

}

//////////////////////////////////////////////////////////
// update cache of 1 tile around the latest access point
// to prepare for upcoming reads

void TerrainHtServer::_updateCache()

{

  if (_latestLat == -9999.0 || _latestLon == -9999.0) {
    // no activity yet
    return;
  }

  _readForCache(_latestLat + 1.0, _latestLon - 1.0);
  _readForCache(_latestLat + 1.0, _latestLon);
  _readForCache(_latestLat + 1.0, _latestLon + 1.0);

  _readForCache(_latestLat, _latestLon - 1.0);
  _readForCache(_latestLat, _latestLon + 1.0);

  _readForCache(_latestLat - 1.0, _latestLon - 1.0);
  _readForCache(_latestLat - 1.0, _latestLon);
  _readForCache(_latestLat - 1.0, _latestLon + 1.0);
  
}

//////////////////////////////////////////////////////////
// read in a tile for the cache
// returns 0 on success, -1 on failure

int TerrainHtServer::_readForCache(double lat, double lon)

{

  // compute tile indices

  int ilat = (int) (lat - -90.0);
  int ilon = (int) (lon - -180.0);

  // check for out of bounds

  if (ilat < 0) return 0;
  if (ilat > nLat - 1) return 0;
  if (ilon < 0) return 0;;
  if (ilon > nLon - 1) return 0;

  // read tile for cache

  if (_tiles[ilat][ilon]->readForCache()) {
    cerr << "ERROR - TerrainHtServer::getHt()" << endl;
    cerr << "  Cannot read for cache, lat, lon: " << lat << ", " << lon << endl;
    cerr << "  Tile indices: ilat, ilon: " << ilat << ", " << ilon << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////
// free up tile memory that has not been used recently

void TerrainHtServer::_freeTileMemory()

{

  time_t now = time(NULL);

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      time_t ltime = _tiles[ilat][ilon]->getLatestAccessTime();
      if (ltime > 0) {
        double secsSinceLastAccess = (double) now - (double) ltime;
        if (secsSinceLastAccess > _params.secs_before_freeing_memory) {
          _tiles[ilat][ilon]->freeHtAndWaterArrays();
        }
      } // if (ltime > 0)
    } // ilon
  } // ilat

}



