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
// Egm2008.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Geoid height correction for EGM 2008
//
// See:
// https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <radar/Egm2008.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

// Constructor

Egm2008::Egm2008()
{
    
  _debug = false;
  _verbose = false;

  _geoidBuf = NULL;

  // initialize assuming 2.5 minute data

  int nPtsPerDeg = 24;
  _gridRes = 1.0 / nPtsPerDeg;
  _nLat = 180 * nPtsPerDeg + 1;
  _nLon = 360 * nPtsPerDeg;
  _nPoints = _nLat * _nLon;
  
}

// destructor

Egm2008::~Egm2008()
  
{

  if (_geoidBuf != NULL) {
    delete[] _geoidBuf;
  }

}

/////////////////////////////////////////////////////////////
// read in the Geoid corrections file
// for 2008 version of EGM

int Egm2008::readGeoid(const string &path)

{

  Path pathParts(path);

  // If a netcdf file or MDV file, read in using MDV

  if (pathParts.getExt() == "nc" ||
      pathParts.getExt() == "mdv") {

    if (_readMdvNetcdf(path) == 0) {
      return 0;
    }
    
  }
    
  if (_readFortranBinaryFile(path)) {
    return -1;
  }

  return 0;

}
  
/////////////////////////////////////////////////////////////
// read in the Geoid corrections file
// for 2008 version of EGM
// from an MDV NetCDF file
// This file would be created using the Egm2Mdv app.

int Egm2008::_readMdvNetcdf(const string &path)

{

  _mdvx.setReadPath(path);
  if (_mdvx.readVolume()) {
    cerr << "ERROR - Egm2008::_readMdvNetcdf" << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  }
  
  MdvxField *geoidFld = _mdvx.getFieldByNum(0);
  if (geoidFld == NULL) {
    cerr << "ERROR - Egm2008::_readMdvNetcdf" << endl;
    cerr << "  Cannot find geoid correction field" << endl;
    cerr << "  Path: " << path << endl;
    return -1;
  }

  _geoidM = (const fl32*) geoidFld->getVol();
  const Mdvx::field_header_t &fhdr = geoidFld->getFieldHeader();
  _gridRes = fhdr.grid_dx;
  _minLat = fhdr.grid_miny;
  _minLon = fhdr.grid_minx;
  _nLat = fhdr.ny;
  _nLon = fhdr.nx;
  _nPoints = _nLat * _nLon;

  return 0;

}

  
/////////////////////////////////////////////////////////////
// read in the Geoid corrections file
// for 2008 version of EGM
// from a binary file with FORTRAN record markers

int Egm2008::_readFortranBinaryFile(const string &path)

{
  
  // get file size
  
  struct stat fstat;
  if (ta_stat(path.c_str(), &fstat)) {
    int errNum = errno;
    cerr << "ERROR - Egm2008::readGeoid" << endl;
    cerr << "  Cannot stat egm file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  off_t nbytes = fstat.st_size;
  
  // assume 2.5 minute file to start
  
  int nPtsPerDeg = 24;
  if (nbytes > 149368328) {
    // 1 minute file
    nPtsPerDeg = 60;
  }
  _gridRes = 1.0 / nPtsPerDeg;

  _nLat = 180 * nPtsPerDeg + 1;
  _nLon = 360 * nPtsPerDeg;
  _nPoints = _nLat * _nLon;
  ui32 recLen = _nLon * sizeof(fl32);

  if (_debug) {
    cerr << "==>> nPtsPerDeg: " << nPtsPerDeg << endl;
    cerr << "==>> _gridRes: " << _gridRes << endl;
    cerr << "==>> _nLat: " << _nLat << endl;
    cerr << "==>> _nLon: " << _nLon << endl;
    cerr << "==>> _nPoints: " << _nPoints << endl;
    cerr << "==>> recLen: " << recLen << endl;
  }
  
  int nexpected = _nPoints * sizeof(fl32) + _nLat * 2 * sizeof(ui32);
  if (nbytes != nexpected) {
    cerr << "ERROR - Egm2008::readGeoid" << endl;
    cerr << "ERROR - bad egm2008 file: " << path << endl;
    cerr << "  expected nbytes: " << nexpected << endl;
    cerr << "  file size nbytes: " << nbytes << endl;
    return -1;
  }

  // open elevation data file
  
  FILE *egmFile;
  if ((egmFile = fopen(path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Egm2008::readGeoid" << endl;
    cerr << "  cannot open egm file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // geoid data in meters

  if (_geoidBuf != NULL) {
    delete[] _geoidBuf;
  }
  _geoidBuf = new fl32[_nPoints];

  int npts = 0;
  bool mustSwap = false;
  
  // read through the file, a fortran record at a time
  
  while (!feof(egmFile)) {
    
    // read starting fortran record len
    
    ui32 recLenStart;
    if (fread(&recLenStart, sizeof(recLenStart), 1, egmFile) != 1) {
      int errNum = errno;
      if (feof(egmFile)) {
        break;
      }
      cerr << "ERROR - Egm2008::readGeoid" << endl;
      cerr << "  cannot read start fort rec len from egm file: " 
           << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      return -1;
    }
    if (recLenStart != recLen) {
      recLenStart = BE_from_ui32(recLenStart);
      mustSwap = true;
    }
    if (recLenStart != recLen) {
      cerr << "ERROR - Egm2008::readGeoid" << endl;
      cerr << "  egm2008 file: " << path << endl;
      cerr << "  bad recLenStart: " << recLenStart << endl;
      fclose(egmFile);
      return -1;
    }
    if (_verbose) {
      cerr << "Start fort rec len: " << recLenStart << endl;
    }
    
    // read in data
    
    int nFl32InRec = recLenStart / sizeof(fl32);
    fl32 *buf = new fl32[nFl32InRec];
    if (fread(buf, sizeof(fl32), nFl32InRec, egmFile) != (size_t) nFl32InRec) {
      int errNum = errno;
      cerr << "ERROR - Egm2008::readGeoid" << endl;
      cerr << "  cannot read data from egm file: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      delete[] buf;
      return -1;
    }
    
    // swap as needed

    if (mustSwap) {
      BE_from_array_32(buf, nFl32InRec * sizeof(fl32));
    }

    // copy data into main array
    
    for (off_t ii = 0; ii < nFl32InRec; ii++) {
      _geoidBuf[npts] = buf[ii];
      npts++;
    }
    delete[] buf;
    
    // read ending fortran record len
    
    ui32 recLenEnd;
    if (fread(&recLenEnd, sizeof(recLenEnd), 1, egmFile) != 1) {
      int errNum = errno;
      cerr << "ERROR - Egm2008::readGeoid" << endl;
      cerr << "  cannot read end fort rec len from egm file: " 
           << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      return -1;
    }
    if (mustSwap) {
      recLenEnd = BE_from_ui32(recLenEnd);
    }
    if (recLenEnd != recLen) {
      cerr << "ERROR - Egm2008::readGeoid" << endl;
      cerr << "  egm2008 file: " << path << endl;
      cerr << "  bad recLenEnd: " << recLenEnd << endl;
      fclose(egmFile);
      return -1;
    }
    if (_verbose) {
      cerr << "End fort rec len: " << recLenEnd << endl;
    }
    
  } // while
  
  // close file
  
  fclose(egmFile);

  if (_debug) {
    cerr << "Read npts: " << npts << endl;
  }
  
  // print out in debug mode
  
  if (_verbose) {
    cerr << "========== egm2008 file =====>> : " << path << endl;
    int ipt = 0;
    for (int ilat = 0; ilat < _nLat; ilat++) {
      double lat = 90.0 - ilat * _gridRes;
      for (int ilon = 0; ilon < _nLon; ilon++, ipt++) {
        double lon = ilon * _gridRes;
        if (lon > 180.0) {
          lon -= 360.0;
        }
        fprintf(stderr, "lat, lon, geoidM: %10.5f  %10.5f  %10.5f\n", 
                lat, lon, _geoidBuf[ipt]);
      }
    } // ilat
  }

  // reorder the data so it starts at the SW corner

  _reorderGeoidLats();
  _reorderGeoidLons();

  _geoidM = _geoidBuf;
  _minLat = -90.0;
  _minLon = -180.0;

  return 0;

}

/////////////////////////////////////////////////////////////
// Get the closest geoid correction for a given lat/lon

double Egm2008::getClosestGeoidM(double lat, double lon) const
  
{

  if (_geoidM == NULL) {
    // not yet initialized
    cerr << "ERROR - Egm2008::getClosestGeoidM" << endl;
    cerr << "  Not initialized, returning 0" << endl;
    return 0.0;
  }
  
  int ilat = (int) (((90.0 - lat) / _gridRes) + 0.5);
  int ilon = (int) ((lon / _gridRes) + 0.5);
  if (lon < 0) {
    ilon = (int) (((lon + 360.0) / _gridRes) + 0.5);
  }
  
  int index = ilon + ilat * _nLon;
  if (index > _nPoints - 1) {
    cerr << "ERROR - Egm2008::getClosestGeoidM" << endl;
    cerr << "  lat, lon: " << lat << ", " << lon << endl;
    cerr << "  Bad index: " << index << endl;
    cerr << "  Max allowable index: " << _nPoints - 1 << endl;
    cerr << "  Returning 0.0" << endl;
    return 0.0;
  }

  cerr << "222222222222222222222222222222222222222" << endl;
  cerr << "lat: " << lat << endl;
  cerr << "lon: " << lon << endl;
  cerr << "ilat: " << ilat << endl;
  cerr << "ilon: " << ilon << endl;
  cerr << "index: " << index << endl;
  cerr << "closest: " << _geoidM[index] << endl;
  cerr << "222222222222222222222222222222222222222" << endl;

  return _geoidM[index];

}

/////////////////////////////////////////////////////////////
// Get the interpolated geoid correction for a given lat/lon

double Egm2008::getInterpGeoidM(double lat, double lon) const
  
{

  if (_geoidM == NULL) {
    // not yet initialized
    cerr << "ERROR - Egm2008::getInterpGeoidM" << endl;
    cerr << "  Not initialized, returning 0" << endl;
    return 0.0;
  }

  // compute latitude indices on either side of the point
  
  int ilat0 = getLatIndexSouth(lat);
  int ilat1 = getLatIndexNorth(lat);
  
  // compute longitude indices on either side of the point

  if (lon < 0) {
    lon += 360.0;
  } else if (lon > 360) {
    lon -= 360.0;
  }
  int ilon0 = getLonIndexWest(lon);
  int ilon1 = getLonIndexEast(lon);

  // get lat and lon for the indices
  
  double lat0 = getLat(ilat0);
  double lat1 = getLat(ilat1);
  double latFraction = (lat - lat0) / _gridRes;
  
  double lon0 = getLon(ilon0);
  double lon1 = getLon(ilon1);
  double lonFraction = (lon - lon0) / _gridRes;

  // interp starting with latitude

  double g00 = _geoidM[ilon0 + ilat0 * _nLon];
  double g01 = _geoidM[ilon0 + ilat1 * _nLon];
  
  double g10 = _geoidM[ilon1 + ilat0 * _nLon];
  double g11 = _geoidM[ilon1 + ilat1 * _nLon];

  double g0 = g00 + (g01 - g00) * latFraction;
  double g1 = g10 + (g11 - g10) * latFraction;

  double gg = g0 + (g1 - g0) * lonFraction;

  cerr << "111111111111111111111111111111111111111" << endl;
  cerr << "_nLat, _nLon: " << _nLat << ", " << _nLon << endl;
  cerr << "_gridRes: " << _gridRes << endl;
  cerr << "lat, lon: " << lat << ", " << lon << endl;
  cerr << "ilat0, ilat1: " << ilat0 << ", " << ilat1 << endl;
  cerr << "ilon0, ilon1: " << ilon0 << ", " << ilon1 << endl;
  cerr << "lat0, lat1: " << lat0 << ", " << lat1 << endl;
  cerr << "lon0, lon1: " << lon0 << ", " << lon1 << endl;
  cerr << "latFraction: " << latFraction << endl;
  cerr << "lonFraction: " << lonFraction << endl;
  cerr << "g00, g01: " << g00 << ", " << g01 << endl;
  cerr << "g10, g11: " << g10 << ", " << g11 << endl;
  cerr << "g0, g1: " << g0 << ", " << g1 << endl;
  cerr << "gg: " << gg << endl;
  cerr << "closest: " << getClosestGeoidM(lat, lon) << endl;
  cerr << "111111111111111111111111111111111111111" << endl;

  return gg;

}

/////////////////////////////////////////////////////////
// reorder the geoid latitude
// the original data is north to south
// we need the data south to north, for normal coordinates

void Egm2008::_reorderGeoidLats()
  
{

  // create an array for a line of data at a time

  fl32 *rowData = new fl32[_nLon];

  // loop through the top half of the rows

  size_t nBytesRow = _nLon * sizeof(fl32);
  
  for (int irow = 0; irow < _nLat / 2; irow++) {

    // make a copy of row data in northern half
    
    memcpy(rowData,
           _geoidBuf + irow * _nLon,
           nBytesRow);
    
    // copy from southern half to northern half

    memcpy(_geoidBuf + irow * _nLon,
           _geoidBuf + (_nLat - 1 - irow) * _nLon,
           nBytesRow);

    // copy from northern half to southern half
    
    memcpy(_geoidBuf + (_nLat - 1 - irow) * _nLon,
           rowData,
           nBytesRow);

  }

  // clean up

  delete[] rowData;

}


///////////////////////////////////////////////////////////
// reorder the geoid longitude
// the original data is 0 to 360
// reorder from -180 to +180

void Egm2008::_reorderGeoidLons()
  
{

  // create arrays

  size_t nLonHalf = _nLon / 2;
  fl32 *rowData = new fl32[_nLon];
  size_t nBytesRow = _nLon * sizeof(fl32);
  size_t nBytesHalf = nLonHalf * sizeof(fl32);
  
  // loop through the top half of the rows
  
  
  for (int irow = 0; irow < _nLat; irow++) {

    // make a copy of east half of row data
    
    memcpy(rowData,
           _geoidBuf + irow * _nLon + nLonHalf,
           nBytesHalf);
    
    // make a copy of west half of row data
    
    memcpy(rowData + nLonHalf,
           _geoidBuf + irow * _nLon,
           nBytesHalf);
    
    // copy from row data back into place

    memcpy(_geoidBuf + irow * _nLon,
           rowData,
           nBytesRow);

  }

  // clean up
  
  delete[] rowData;

}


