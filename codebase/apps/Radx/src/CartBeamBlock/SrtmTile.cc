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
// SrtmTile.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// SrtmTile handles the terrain information for a single
// 1 deg x 1 deg segment of the global grid.
// 
////////////////////////////////////////////////////////////////

#include "SrtmTile.hh"
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <iostream>
#include <cmath>
#include <cerrno>

using namespace std;
// TaThread::SafeMutex *SrtmTile::_globalMutex = NULL;

// Constructor

SrtmTile::SrtmTile(const string &demDir,
                   double centerLat,
                   double centerLon,
                   bool debug /* = false */) :
        _debug(debug),
        _demDir(demDir),
        _centerLat(centerLat),
        _centerLon(centerLon)
        
{

  // force consistency with globe

  if (_centerLat > 90) {
    _centerLat = 89.5;
  }
  if (_centerLat < -90) {
    _centerLat = -89.5;
  }

  while (_centerLon >= 180) {
    _centerLon -= 360.0;
  }
  while (_centerLon < -180) {
    _centerLon += 360.0;
  }

  _llLat = (int) (_centerLat);
  _llLon = (int) (_centerLon);

  // _latestAccessTime = 0;

  // if (_globalMutex == NULL) {
  //   _globalMutex = new TaThread::SafeMutex;
  // }

}

////////////////////////////////////////////////
// destructor

SrtmTile::~SrtmTile()

{
  freeHtArray();
}

////////////////////////////////////////////////
// free height array

void SrtmTile::freeHtArray()

{
  
  // lock mutex - will unlock going out of scope
    
  TaThread::LockForScope lock(&_localMutex);
  _htArray.clear();

}

////////////////////////////////////////////////
// get terrain ht for a point
// returns 0 on success, -1 on failure
// sets terrainHtM

int SrtmTile::getHt(double lat, double lon, int16_t &terrainHtM)

{

  // lock mutex - will unlock going out of scope
  
  TaThread::LockForScope lock(&_localMutex);
  
  if (_htArray.empty()) {
    if (_readFromFile()) {
      cerr << "ERROR - SrtmTile::getHt" << endl;
      cerr << "  Cannot read ht data from file" << endl;
      terrainHtM = -9999.0;
      return -1;
    }
  }

  // compute offsets

  int dlat = (int) _ny - ((lat - _llLat) / _dy + 0.5);
  int dlon = (int) ((lon - _llLon) / _dx + 0.5);

  if (dlat < 0) {
    dlat = 0;
  } else if (dlat > _ny - 1) {
    dlat = _ny - 1;
  }

  if (dlon < 0) {
    dlon = 0;
  } else if (dlon > _nx - 1) {
    dlon = _nx - 1;
  }

  terrainHtM = _htArray[dlat][dlon];

  return 0;

}

/////////////////////////////////////////
// read height data from the file
// populate array

int SrtmTile::_readFromFile()
  
{

  // get the DEM file path, and other info

  if (_findFile()) {
    return -1;
  }

  // get file status, and set properties
  
  struct stat fstat;
  if (ta_stat(_demFilePath, &fstat)) {
    int errNum = errno;
    cerr << "ERROR - SrtmTile::_readFromFile" << endl;
    cerr << "  Cannot get file properties: " << _demFilePath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  off_t fileSizeBytes = fstat.st_size;
  double nValsFile = fstat.st_size / sizeof(int16_t);
  _ptsPerDeg = (int) (sqrt(nValsFile));
  _gridRes = 1.0 / (double) _ptsPerDeg;

  if (_ptsPerDeg != 1201 && _ptsPerDeg != 3601) {
    cerr << "ERROR - SrtmTile::_readFromFile" << endl;
    cerr << "  File is wrong size (byts): " << fileSizeBytes << endl;
    cerr << "  _ptsPerDeg: " << _ptsPerDeg << endl;
    cerr << "  Should be 1201 (SRTM3) or 3601 (SRTM1)" << endl;
    return -1;
  }

  _nx = _ptsPerDeg;
  _ny = _ptsPerDeg;
  _dx = 1.0 / (_nx - 1);
  _dy = 1.0 / (_ny - 1);
  
  // open file
  
  FILE *in;
  if ((in = fopen(_demFilePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SrtmTile::_readFromFile" << endl;
    cerr << "  Cannot open file: " << _demFilePath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  
  if (_debug) {
    cerr << "SrtmTile, lat, lon: " << _centerLat << ", " << _centerLon << endl;
    cerr << "  Reading in DEM file; " << _demFilePath << endl;
  }

  // alloc array

  _htArray.alloc(_ny, _nx);
  
  // read in data

  size_t npts = _ny * _nx;
  if (fread(_htArray.dat1D(), sizeof(int16_t), npts, in) != npts) {
    int errNum = errno;
    cerr << "ERROR - SrtmTile::_readFromFile" << endl;
    cerr << "  Cannot read tile from file: " << _demFilePath << endl;
    cerr << strerror(errNum) << endl;
    fclose(in);
    return -1;
  }

  // byte swap
  
  BE_to_array_16(_htArray.dat1D(), npts * sizeof(int16_t));
  
  // close file
  
  fclose(in);

  return 0;

}

////////////////////////////////////////////////
// finds the DEM file for this square

int SrtmTile::_findFile()
  
{
  
  if ((fabs(_centerLat) > 90.0) || (fabs(_centerLon) > 180.0)) {
    cerr << "ERROR - cannot locate file for lat/lon: " 
         << _centerLat << ", "
         << _centerLon << endl;
    return -1;
  }
  
  _llLat = (int) floor(_centerLat);
  _llLon = (int) floor(_centerLon);
  
  char latChar, lonChar;
  
  if (_llLat >= 0) {
    latChar = 'N';
  } else {
    latChar = 'S';
  }
  
  if (_llLon >= 0) {
    lonChar = 'E';
  } else {
    lonChar = 'W';
  }
  
  snprintf(_demFilePath, MAX_PATH_LEN,
           "%s%s%c%02d%c%03d.hgt",
           _demDir.c_str(), PATH_DELIM,
           latChar, abs(_llLat),
           lonChar, abs(_llLon));
  
  return 0;
  
}

