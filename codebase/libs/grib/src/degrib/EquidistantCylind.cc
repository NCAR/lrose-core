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
///////////////////////////////////////////////////
// EquidistantCylind - Equidistant Cylindrical grid
//     definition - also called Latitude/Longitude
//     or Plate Carree projection grid 
//
//////////////////////////////////////////////////

#include <grib/EquidistantCylind.hh>
#include <grib/constants.h>
#include <iostream>
using namespace std;


EquidistantCylind::EquidistantCylind()
:GDS()
{
  _directionIncsGiven = false;

  _earthSpherical = true;

  _uvRelToGrid = false;
}


EquidistantCylind::~EquidistantCylind() {

}


int
EquidistantCylind::unpack (ui08 *gdsPtr) {

   // section common to all projections:

   // Length in bytes of the section
   _nBytes = _upkUnsigned3( gdsPtr[0], gdsPtr[1], gdsPtr[2] );

#if	0
   if(_nBytes > 44) {
     cout << "ERROR: Possible corrupt record. GDS size in bytes is " <<
       _nBytes << endl << flush;
     return( GRIB_FAILURE );
   }
#endif

  // get the coordinate parameters 
   _numVertical = (int)gdsPtr[3]; 
   _verticalOrPoints = (int)gdsPtr[4]; 

   // Get the projection type
   _projType = (int)gdsPtr[5];

  // LATLON projection specific parameters

  _ni = _upkUnsigned2(gdsPtr[6], gdsPtr[7]);  // along latitude circle
  _nj = _upkUnsigned2(gdsPtr[8], gdsPtr[9]);  // along longitude meridian
  _projection.setGridDims(_ni, _nj, _projection.getNz());

  // Latitude of first grid point (leftmost bit set for south latitude
  _la1 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[10], gdsPtr[11], gdsPtr[12]);
  // Longitude of first grid point (leftmost bit set for west longitude
  _lo1 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[13], gdsPtr[14], gdsPtr[15]);

  // la1 is always correct here. Where does it get reversed?
  _projection.setGridMins(_lo1, _la1, _projection.getMinz());

  //
  // Resolution and component flags
  //
  _resolutionFlag = gdsPtr[16];
  if (_resolutionFlag & 128 )
    _directionIncsGiven = true;

  if (_resolutionFlag & 64 ) {
     _earthSpherical = false;
  }

  if (_resolutionFlag & 8 )
     _uvRelToGrid = true;


  // Latitude of last grid point (leftmost bit set for south latitude
  _la2 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[17], gdsPtr[18], gdsPtr[19]);
  // Longitude of last grid point (leftmost bit set for west longitude
  _lo2 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[20], gdsPtr[21], gdsPtr[22]);

  // Longitudinal Direction Increment  (undefined - all bits set to 1)
  _di = GRID_SCALE_FACTOR * (double) _upkUnsigned2(gdsPtr[23], gdsPtr[24]);

  // Latitudinal Direction Increment (undefined - all bits set to 1)t
  _dj = GRID_SCALE_FACTOR * (double) _upkUnsigned2(gdsPtr[25], gdsPtr[26]);

  _projection.setGridDeltas(_di, _dj, _projection.getDz());
  
  // Scanning mode flags
  // probably want to actually assign these flag values
  _scanModeFlag = gdsPtr[27];

  //
  // data ordering
  //
  if ((_scanModeFlag & 32) == 0) {
    _dataOrder = DO_XY;
  }
  else {
    _dataOrder = DO_YX;
  }

  //
  // grid orientation
  //
  if ((_scanModeFlag & 192) == 64) {
    _gridOrientation = GO_SN_WE;
  }
  else if ((_scanModeFlag & 192) == 0) {
    _gridOrientation = GO_NS_WE;
  }
  else if ((_scanModeFlag & 192) == 192) {
    _gridOrientation = GO_SN_EW;
  }
  else if ((_scanModeFlag & 192) == 128) {
    _gridOrientation = GO_NS_EW;
  }

  unpackPtsPerRow(gdsPtr);

  return( GRIB_SUCCESS );
}

int
EquidistantCylind::pack (ui08 *gdsPtr) 
{

  // Length in bytes of the section

  _pkUnsigned3(_nBytes, &(gdsPtr[0]));

  // vertical coordinate parameters
  gdsPtr[3] = (ui08)_numVertical;
  gdsPtr[4] = (ui08)_verticalOrPoints;

  // Projection type
  gdsPtr[5] = (ui08)_projType;
  
  // Number of points in X and Y

  _pkUnsigned2(_projection.getNx(), &(gdsPtr[6]));
  _pkUnsigned2(_projection.getNy(), &(gdsPtr[8]));

  // first point latitude and longitude
  //
  //  units millidegrees (1000 * degrees)
  //  lat 1: range 0 to 90,000; bit 1 (leftmost) set to 1 for 
  //         south latitude
  //  lon 1: range 0 to 360,000; bit 1 (leftmost) set to 1 for 
  //         west longitude
  _pkSigned3((int)(getFirstLat() / DEGREES_SCALE_FACTOR), &(gdsPtr[10]));
  if(getFirstLat() < 0.0) 
    {
      gdsPtr[10] |= 255;
    }
  _pkSigned3((int)((getFirstLon() + 360.0) / DEGREES_SCALE_FACTOR),
	     &(gdsPtr[13]));
  if(getFirstLon() < 0.0) 
    {
      gdsPtr[13] |= 255;
    }

  //
  // resolution and component flag -- hard code to 136
  // the value implies:
  //     1) direction increments are given
  //     2) earth assumed spherical with radius = 6367.47
  //     3) u and v components of vector quantities resolved relative to the
  //        defined grid in the direction of increasing x and y
  //
  _resMode = 136;
  gdsPtr[16] = (ui08)_resMode;
    

  // last point latitude and longitude
  //
  // same units, value range and bit one as first point
  //
  _pkSigned3((int)(getLastLat() / DEGREES_SCALE_FACTOR), &(gdsPtr[17]));
  if(getLastLat() < 0.0) 
    {
      gdsPtr[17] |= 255;
    }
  _pkSigned3((int)((getLastLon() + 360.0) / DEGREES_SCALE_FACTOR),
	     &(gdsPtr[20]));
  if(getLastLon() < 0.0) 
    {
      gdsPtr[20] |= 255;
    }
    
  // dx and dy

  _pkSigned2((int)(_projection.getDx() / DEGREES_SCALE_FACTOR), &(gdsPtr[23]));
  _pkSigned2((int)(_projection.getDy() / DEGREES_SCALE_FACTOR), &(gdsPtr[25]));

  // assume projection is from north pole

  gdsPtr[27] = _setScanMode();

  return GRIB_SUCCESS;
}

void EquidistantCylind::print(FILE *stream) const
{
  GDS::print(stream);

  fprintf(stream, "\nEquidistantCylind Projection Section:\n");
  fprintf(stream, "    Equidistant Latitude/longitude projection\n");
  fprintf(stream, "---------------------------------------------\n");
  fprintf(stream, "Number of points along latitude circle %d\n", _ni);
  fprintf(stream, "Number of points along longitude meridian %d\n", _nj);
  fprintf(stream, "Latitude of first grid point %f\n", _la1);
  fprintf(stream, "Longitude of first grid point %f\n", _lo1);
  fprintf(stream, "Resolution flag byte %d\n", _resolutionFlag);
  if (_directionIncsGiven == true)
    fprintf(stream, "    Direction increments given\n");
  else
    fprintf(stream, "    Direction increments not given\n");

  if (_earthSpherical == true)
    fprintf(stream, "    Earth considered spherical - radius = 6367.47km\n");
  else
    fprintf(stream, "    Earth considered oblate spheroid - size = 6378.160 km, 6356.775 km, f=1/296.0\n");

  if (_uvRelToGrid == true) {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to easterly\n");
    fprintf(stream, "     and northerly directions\n");
  }
  else {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to the defined\n");
    fprintf(stream, "    grid in the direction of increasing x and y (or i and j) coordinates respectively\n");
  }
  fprintf(stream, "Latitude of last grid point %f\n", _la2);
  fprintf(stream, "Longitude of last grid point %f\n", _lo2);
  fprintf(stream, "Longitudinal Direction Increment %f\n", _di);
  fprintf(stream, "Latitudinal Direction Increment %f\n", _dj);
  fprintf(stream, "Scanning mode flags %d\n", _scanModeFlag);
  if ((_scanModeFlag & 32) == 0) {
    fprintf(stream, "    Data Order is X to Y\n");
  }
  else {
    fprintf(stream, "    Data Order is Y to X\n");
  }

  //
  // grid orientation
  //
  if ((_scanModeFlag & 192) == 64) {
    fprintf(stream, "    Grid orientation is South to North, West to East\n");
  }
  else if ((_scanModeFlag & 192) == 0) {
    fprintf(stream, "    Grid orientation is North to South, West to East\n");
  }
  else if ((_scanModeFlag & 192) == 192) {
    fprintf(stream, "    Grid orientation is South to North, East to West\n");
  }
  else if ((_scanModeFlag & 192) == 128) {
    fprintf(stream, "    Grid orientation is North to South, East to West\n");
  }
}


void EquidistantCylind::print(ostream &stream) const
{
  GDS::print(stream);

  stream << endl << "EquidistantCylind Projection Section:" << endl;
  stream << "    Equidistant Latitude/longitude projection" << endl;
  stream << "---------------------------------------------" << endl;
  stream << "Number of points along latitude circle " <<  _ni << endl;
  stream << "Number of points along longitude meridian " <<  _nj << endl;
  stream << "Latitude of first grid point " <<  _la1 << endl;
  stream << "Longitude of first grid point " <<  _lo1 << endl;
  stream << "Resolution flag byte " <<  _resolutionFlag << endl;
  if (_directionIncsGiven == true)
    stream << "    Direction increments given" << endl;
  else
    stream << "    Direction increments not given" << endl;

  if (_earthSpherical == true)
    stream << "    Earth considered spherical - radius = 6367.47km" << endl;
  else
    stream << "    Earth considered oblate spheroid - size = 6378.160 km, 6356.775 km, f=1/296.0" << endl;

  if (_uvRelToGrid == true) {
    stream << "    u- and v- components of vector quantities resolved relative to easterly" << endl;
    stream << "     and northerly directions" << endl;
  }
  else {
    stream << "    u- and v- components of vector quantities resolved relative to the defined" << endl; 
    stream << "    grid in the direction of increasing x and y (or i and j) coordinates respectively" << endl;
  }
  stream << "Latitude of last grid point " <<  _la2 << endl;
  stream << "Longitude of last grid point " <<  _lo2 << endl;
  stream << "Longitudinal Direction Increment " <<  _di << endl;
  stream << "Latitudinal Direction Increment " <<  _dj << endl;
  stream << "Scanning mode flags " <<  _scanModeFlag << endl;
  if ((_scanModeFlag & 32) == 0) {
    stream << "    Data Order is X to Y" << endl;
  }
  else {
    stream << "    Data Order is Y to X" << endl;
  }

  //
  // grid orientation
  //
  if ((_scanModeFlag & 192) == 64) {
    stream << "    Grid orientation is South to North, West to East" << endl;
  }
  else if ((_scanModeFlag & 192) == 0) {
    stream << "    Grid orientation is North to South, West to East" << endl;
  }
  else if ((_scanModeFlag & 192) == 192) {
    stream << "    Grid orientation is South to North, East to West" << endl;
  }
  else if ((_scanModeFlag & 192) == 128) {
    stream << "    Grid orientation is North to South, East to West" << endl;
  }
}


void EquidistantCylind::setRegular(int numRows, int numColumns)
{
  GDS::setRegular(numRows, numColumns);

  // set class members here
  _ni = numColumns;
  if (_lo2 < _lo1) {
    // take care of crossing the dateline
    _di = (360.0 + _lo2 - _lo1) / (_ni - 1);
  } else {
    _di = (_lo2 - _lo1) / (_ni - 1);
  }

  // adjust the X-increment of the projection
  double dx, dy, dz;
  _projection.getGridDeltas(dx, dy, dz);
  _projection.setGridDeltas(_di, dy, dz);
}

