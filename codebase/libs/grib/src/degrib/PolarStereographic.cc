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
// Polar Stereographic - polar stereographic grid
// Author:  Carl Drews
// Created:  October 2005
//
//////////////////////////////////////////////////

#include <grib/PolarStereographic.hh>
#include <grib/constants.h>
#include <iostream>
// #include <euclid/euclid_macros.h>
using namespace std;


PolarStereographic::PolarStereographic()
:GDS()
{
  _directionIncsGiven = false;
  _earthSpherical = true;
  _uvRelToGrid = false;
}


PolarStereographic::~PolarStereographic() {

}

void
PolarStereographic::setProjection(const Pjg &new_projection)
  
{
  GDS::setProjection(new_projection);

  _lov = _projection.getRotation();
  _nx = _projection.getNx();
  _ny = _projection.getNy();
  _la1 = _projection.getOriginLat();
  _lo1 = _projection.getOriginLon();
  _dx = _projection.getDx();
  _dy = _projection.getDy();

}


int
PolarStereographic::unpack (ui08 *gdsPtr) {

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

  // POLAR_STEREO projection specific parameters

  _nx = _upkUnsigned2(gdsPtr[6], gdsPtr[7]);  // num points along x-axis
  _ny = _upkUnsigned2(gdsPtr[8], gdsPtr[9]);  // num points along y-axis
  _projection.setGridDims(_nx, _ny, _projection.getNz());

  // Latitude of first grid point (leftmost bit set for south latitude
  _la1 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[10], gdsPtr[11], gdsPtr[12]);
  // Longitude of first grid point (leftmost bit set for west longitude
  _lo1 = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[13], gdsPtr[14], gdsPtr[15]);

  _projection.setGridMins(_lo1, _la1, _projection.getMinz());
  _originLat = _la1;
  _originLon = _lo1;

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


  // orientation of the grid
  _lov = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[17], gdsPtr[18], gdsPtr[19]);

  // x-direction grid length in km
  _dx = GRID_SCALE_FACTOR * (double) _upkUnsigned3(gdsPtr[20], gdsPtr[21], gdsPtr[22]);

  // y-direction grid length in km
  _dy = GRID_SCALE_FACTOR * (double) _upkUnsigned3(gdsPtr[23], gdsPtr[24], gdsPtr[25]);

  _projection.setGridDeltas(_dx, _dy, _projection.getDz());
  
  // projection center flag
  _projectionCenterFlag = gdsPtr[26];
  PjgTypes::pole_type_t pole = PjgTypes::POLE_NORTH;
  if (_projectionCenterFlag & 1) {
    pole = PjgTypes::POLE_SOUTH;
  }

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

  // initialize the underlying projection object.  Note that the central
  // scale value comes from GRIB using the 60 degree latitude as the point
  // where meters are accurate.  This may need to change for southern pole
  // based projections.
  _projection.initPolarStereo(_lov, pole, 0.9330127,
    _nx, _ny, 0,
    _dx, _dy, 0.0,
    0, 0, 0.0);

  unpackPtsPerRow(gdsPtr);

  return( GRIB_SUCCESS );
}

int
PolarStereographic::pack (ui08 *gdsPtr) 
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

  _pkSigned3((int)(getFirstLat() / DEGREES_SCALE_FACTOR), &(gdsPtr[10]));
  _pkSigned3((int)((getFirstLon() + 360.0) / DEGREES_SCALE_FACTOR),
	     &(gdsPtr[13]));

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

  // orientation of the grid

  _pkSigned3((int)((_lov + 360.0) / DEGREES_SCALE_FACTOR), &(gdsPtr[17]));

  // dx and dy

  _pkSigned3((int)(_projection.getDx() / GRID_SCALE_FACTOR), &(gdsPtr[20]));
  _pkSigned3((int)(_projection.getDy() / GRID_SCALE_FACTOR), &(gdsPtr[23]));

  gdsPtr[27] = _setScanMode();

  return GRIB_SUCCESS;
}


void PolarStereographic::print(FILE *stream) const
{
  GDS::print(stream);

  fprintf(stream, "\nPolarStereographic Projection Section:\n");
  fprintf(stream, "    Polar stereographic projection\n");
  fprintf(stream, "---------------------------------------------\n");
  fprintf(stream, "Number of points along x-axis %d\n", _nx);
  fprintf(stream, "Number of points along y-axis %d\n", _ny);
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

  fprintf(stream, "Orientation of the grid %f\n", _lov);
  fprintf(stream, "X-direction grid length %f\n", _dx);
  fprintf(stream, "Y-direction grid length %f\n", _dy);

  fprintf(stream, "Projection center flag %d\n", _projectionCenterFlag);

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


void PolarStereographic::print(ostream &stream) const
{
  GDS::print(stream);

  stream << endl << "PolarStereographic Projection Section:" << endl;
  stream << "    Polar stereographic projection" << endl;
  stream << "---------------------------------------------" << endl;
  stream << "Number of points along x-axis " <<  _nx << endl;
  stream << "Number of points along y-axis " <<  _ny << endl;
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

  stream << "Orientation of the grid " <<  _lov << endl;
  stream << "X-direction grid length " <<  _dx << endl;
  stream << "Y-direction grid length " <<  _dy << endl;

  stream << "Projection center flag " <<  _projectionCenterFlag << endl;

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


void PolarStereographic::setRegular(int numRows, int numColumns)
{
  GDS::setRegular(numRows, numColumns);

  // set class members here
  _nx = numColumns;
  _ny = numRows;

  // adjust the X-increment of the projection
  double dx, dy, dz;
  _projection.getGridDeltas(dx, dy, dz);
  _projection.setGridDeltas(_dx, dy, dz);
}

