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
// SquareDegree.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// SquareDegree handles the terrain information for a single
// 1 deg x 1 deg segment of the global grid.
// 
////////////////////////////////////////////////////////////////

#include "SquareDegree.hh"
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <iostream>
#include <cmath>
#include <cerrno>

using namespace std;
TaThread::SafeMutex *SquareDegree::_globalMutex = NULL;
const double SquareDegree::GridRes = 1.0 / (double) PtsPerDeg;

// Constructor

SquareDegree::SquareDegree(const Params &params,
                           double centerLat,
                           double centerLon) :
        _params(params),
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

  _ulLatDeg = ((int) floor(_centerLat)) + 1;
  _ulLonDeg = ((int) floor(_centerLon));

  _htArray = NULL;
  _waterArray = NULL;
  _waterAvail = false;
  _latestAccessTime = 0;

  if (_globalMutex == NULL) {
    _globalMutex = new TaThread::SafeMutex;
  }

}

////////////////////////////////////////////////
// destructor

SquareDegree::~SquareDegree()

{

  freeHtAndWaterArrays();

}

////////////////////////////////////////////////
// free height and water arrays

void SquareDegree::freeHtAndWaterArrays()

{
  
  // lock mutex - will unlock going out of scope
    
  TaThread::LockForScope lock(&_localMutex);
  
  if (_htArray != NULL) {
    
    ufree2((void **) _htArray);
    _htArray = NULL;
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Freed up ht data, lat, lon: "
           << _centerLat << ", " << _centerLon << endl;
    }

  }

  if (_waterArray != NULL) {

    ufree2((void **) _waterArray);
    _waterArray = NULL;
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Freed up water data, lat, lon: "
           << _centerLat << ", " << _centerLon << endl;
    }

  }

  _latestAccessTime = 0;


}

////////////////////////////////////////////////
// get terrain ht and water flag for a point
// returns 0 on success, -1 on failure
// sets terrainHtM and isWater args

int SquareDegree::getHt(double lat, double lon,
                        double &terrainHtM, bool &isWater)

{

  // lock mutex - will unlock going out of scope
  
  TaThread::LockForScope lock(&_localMutex);
  
  if (_htArray == NULL) {
    if (_readFromFile()) {
      cerr << "ERROR - SquareDegree::getHt" << endl;
      cerr << "  Cannot read ht data from file" << endl;
      terrainHtM = -9999.0;
      isWater = false;
      return -1;
    }
  }

  // compute offsets

  int dlat = (int) ((_ulLatDeg - lat) / _dy);
  int dlon = (int) ((lon - _ulLonDeg) / _dx);

  if (dlat < 0) {
    dlat =0;
  } else if (dlat > PtsPerDeg - 1) {
    dlat = PtsPerDeg - 1;
  }

  if (dlon < 0) {
    dlon =0;
  } else if (dlon > PtsPerDeg - 1) {
    dlon = PtsPerDeg - 1;
  }

  terrainHtM = _htArray[dlat][dlon];

  if (_waterAvail) {
    if (_waterArray[dlat][dlon] == 1) {
      isWater = true;
    } else {
      isWater = false;
    }
  } else {
    // no water data, water only for 0 terrain
    if (terrainHtM < 1) {
      isWater = true;
    } else {
      isWater = false;
    }
  }
  
  // save latest access time
  
  _latestAccessTime = time(NULL);

  return 0;

}

////////////////////////////////////////////////
// read file to update cache
// returns 0 on success, -1 on failure

int SquareDegree::readForCache()

{

  // read in ht array if needed
  
  if (_htArray == NULL) {
    
    // lock mutex - will unlock going out of scope
    
    TaThread::LockForScope lock(&_localMutex);
    
    if (_readFromFile()) {
      cerr << "ERROR - SquareDegree::readForCache" << endl;
      cerr << "  Cannot read ht data from file" << endl;
      return -1;
    }

  }

  return 0;

}

/////////////////////////////////////////
// read height data from the file
// populate array

int SquareDegree::_readFromFile()
  
{

  // get the DEM file path, and other info

  if (_findFiles()) {
    return -1;
  }

  // open file

  FILE *in;
  if ((in = fopen(_demFilePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SquareDegree::_read" << endl;
    cerr << "  Cannot open file: " << _demFilePath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "SquareDegree, lat, lon: " << _centerLat << ", " << _centerLon << endl;
    cerr << "  Reading in DEM file; " << _demFilePath << endl;
  }

  // compute offset from Upper Left corner of file

  int dLat = (int) (_fileUlLatDeg - _ulLatDeg + 0.5);
  int dLon = (int) (_ulLonDeg - _fileUlLonDeg + 0.5);
  size_t ulOffset = (dLat * _fileNx + dLon) * PtsPerDeg;

  // alloc array

  if (_htArray == NULL) {
    _htArray = (fl32 **) umalloc2(PtsPerDeg, PtsPerDeg, sizeof(fl32));
  }

  // read in data

  size_t offset = ulOffset;
  si16 hts[PtsPerDeg];
  for (int ilat = 0; ilat < PtsPerDeg; ilat++, offset += _fileNx) {
    // read in a lat at a time
    fseek(in, offset * sizeof(si16), SEEK_SET);
    if ((int) fread(hts, sizeof(si16), PtsPerDeg, in) != PtsPerDeg) {
      int errNum = errno;
      cerr << "ERROR - SquareDegree::_read" << endl;
      cerr << "  Cannot read 1 deg of latitude from file: " << _demFilePath << endl;
      cerr << "  ilat: " <<  ilat << endl;
      cerr << "  offset: " <<  offset * sizeof(si16) << endl;
      cerr << strerror(errNum) << endl;
      fclose(in);
      return -1;
    }
    // byte swap
    BE_to_array_16(hts, sizeof(hts));
    // load up ht array
    for (int ilon = 0; ilon < PtsPerDeg; ilon++) {
      if (hts[ilon] == -9999) {
        _htArray[ilat][ilon] = 0;
      } else {
        _htArray[ilat][ilon] = (fl32) hts[ilon];
      }
    }
  } // ilat

  // close file

  fclose(in);

  // read in water data

  if (_readWaterFile(ulOffset)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////
// read water data from the file

int SquareDegree::_readWaterFile(size_t ulOffset)
  
{

  _waterAvail = false;

  // open file

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  Reading in WATER file; " << _waterFilePath << endl;
  }

  // netCDF is not thread safe - so lock global mutex
  
  TaThread::LockForScope lock(_globalMutex);
  
  if (_openNc3File(_waterFilePath)) {
    return -1;
  }

  // get dimensions

  Nc3Dim *latDim, *lonDim;
  if (_readNc3Dim("lat", latDim)) {
    _closeNc3File();
    return -1;
  }
  if (_readNc3Dim("lon", lonDim)) {
    _closeNc3File();
    return -1;
  }
  size_t nLon = lonDim->size();
  size_t nLat = latDim->size();

  // find the water variable

  Nc3Var* waterVar = _ncFile->get_var("water");
  if (waterVar == NULL) {
    cerr << "ERROR - SquareDegree::_readWaterFile" << endl;
    cerr << "  Cannot find variable 'water'" << endl;
    cerr << "  file: " << _ncPathInUse << endl;
    _closeNc3File();
    return -1;
  }
  
  // read in byte array
  
  size_t nPts = nLat * nLon;
  ncbyte *data = new ncbyte[nPts];
  if (!waterVar->get(data, nLat, nLon)) {
    cerr << "ERROR - SquareDegree::_readWaterFile" << endl;
    cerr << "  Cannot read variable 'water'" << endl;
    cerr << "  file: " << _ncPathInUse << endl;
    cerr << _ncErr->get_errmsg() << endl;
    _closeNc3File();
    delete[] data;
    return -1;
  }

  // alloc water array

  if (_waterArray == NULL) {
    _waterArray = (ui08 **) umalloc2(PtsPerDeg, PtsPerDeg, sizeof(ui08));
  }
  memset(*_waterArray, 0, PtsPerDeg * PtsPerDeg);
  _waterAvail = true;

  // copy in data for this square

  for (int ilat = 0; ilat < PtsPerDeg; ilat++) {
    size_t offset = ulOffset + ilat * nLon;
    for (int ilon = 0; ilon < PtsPerDeg; ilon++, offset++) {
      if (offset < nPts) {
        _waterArray[ilat][ilon] = !data[offset];
      }
    }
  }

  delete[] data;
  _closeNc3File();

  return 0;

}


////////////////////////////////////////////////
// finds the DEM and WATER files for this square

int SquareDegree::_findFiles()
  
{
  
  if ((fabs(_centerLat) > 90.0) || (fabs(_centerLon) > 180.0)) {
    cerr << "ERROR - cannot locate file for lat/lon: " 
         << _centerLat << ", "
         << _centerLon << endl;
    return -1;
  }

  _dx = GridRes;
  _dy = GridRes;
 
  char LatChar, LonChar;

  if (_centerLat > -60.0) {
    
    _fileNx = 4800;
    _fileNy = 6000;
    
    LatChar = 'N';
    if (_centerLat <= -10.0) {
      LatChar = 'S';
    }
    
    LonChar = 'E';
    if (_centerLon < 20.0) {
      LonChar = 'W';
    }
    
    _fileUlLatDeg = 0;
    if (_centerLat <= -10.0) _fileUlLatDeg = -10;
    if ((_centerLat > -10.0) && (_centerLat < 40.0)) _fileUlLatDeg = 40;
    if (_centerLat >= 40.0) _fileUlLatDeg = 90;
    
    int ii = (int) floor((_centerLon + 180.0) / 40.0);
    _fileUlLonDeg = -180 + ii * 40;
    
  } else {
    
    _fileNx = 7200;
    _fileNy = 3600;
    
    LatChar = 'S';
    _fileUlLatDeg = -60;
    
    LonChar = 'E';
    if (_centerLon < 60.0) {
      LonChar = 'W';
    }
    
    int ii = (int) floor((_centerLon + 180.0) / 60.0);
    _fileUlLonDeg = -180 + ii * 60;

  }
  
  sprintf(_demFilePath,
          "%s%s%c%03d%c%02d.DEM",
          _params.srtm30_dem_dir,
          PATH_DELIM,
	  LonChar, 
          abs(_fileUlLonDeg),
          LatChar,
          abs(_fileUlLatDeg));
  
  sprintf(_waterFilePath,
          "%s%s%c%03d%c%02d.WATER.nc",
          _params.water_layer_dir,
          PATH_DELIM,
	  LonChar, 
          abs(_fileUlLonDeg),
          LatChar,
          abs(_fileUlLatDeg));
  
  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int SquareDegree::_openNc3File(const string &path)
  
{

  _closeNc3File();
  _ncFile = new Nc3File(path.c_str(), Nc3File::ReadOnly);
  
  // Check that constructor succeeded
  
  if (!_ncFile || !_ncFile->is_valid()) {
    cerr << "ERROR - SquareDegree::_openNc3File" << endl;
    cerr << "  Cannot open file for reading, path: " << path << endl;
    _closeNc3File();
    return -1;
  }

  _ncPathInUse = path;
  
  // Change the error behavior of the netCDF C++ API by creating an
  // Nc3Error object. Until it is destroyed, this Nc3Error object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.

  if (_ncErr == NULL) {
    _ncErr = new Nc3Error(Nc3Error::silent_nonfatal);
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void SquareDegree::_closeNc3File()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }
  
  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

///////////////////////////////////////////
// read a netcdf dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is set

int SquareDegree::_readNc3Dim(const string &name, Nc3Dim* &dim)
  
{
  dim = _ncFile->get_dim(name.c_str());
  if (dim == NULL) {
    cerr << "ERROR - SquareDegree::_readNc3Dim" << endl;
    cerr << "  Cannot read dimension, name: " << name << endl;
    cerr << "  file: " << _ncPathInUse << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }
  return 0;
}

