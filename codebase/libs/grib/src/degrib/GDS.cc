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
// GDS - Grid Description Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
//////////////////////////////////////////////////

#include <euclid/Pjg.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgGrib.hh>
#include <grib/constants.h>
#include <grib/GDS.hh>

const double GDS::DEGREES_SCALE_FACTOR = 0.001;
const double GDS::GRID_SCALE_FACTOR = 0.001;

using namespace std;

GDS::GDS() :
  GribSection(),
  _expectedSize(EXPECTED_SIZE),
  _numVertical(0),
  _verticalOrPoints(NO_VERTICAL_POINTS),
  _projType(0),
  _resMode(0),
  _scanMode(0),
  _lov(0.0),
  _latin1(0.0),
  _latin2(0.0),
  _dataOrder(DO_XY),
  _gridOrientation(GO_SN_WE)
{
  _nBytes = NUM_SECTION_BYTES;
}

GDS::~GDS() 
{
}

void
GDS::setProjection(const Pjg &new_projection)
{
  _projection = new_projection;
  switch (_projection.getProjType())
  {
  case PjgTypes::PROJ_INVALID :
  case PjgTypes::PROJ_FLAT :
  case PjgTypes::PROJ_POLAR_RADAR :
  case PjgTypes::PROJ_MERCATOR :
  case PjgTypes::PROJ_POLAR_ST_ELLIP :
  case PjgTypes::PROJ_RADIAL :
  case PjgTypes::PROJ_UNKNOWN :
    _projType = -1;
    break;
  case PjgTypes::PROJ_STEREOGRAPHIC :
  case PjgTypes::PROJ_POLAR_STEREO :
    _projType = POLAR_STEREOGRAPHIC_PROJ_ID;
    break;
  case PjgTypes::PROJ_LC1 :
  case PjgTypes::PROJ_LC2 :
    _projType = LAMBERT_CONFORMAL_PROJ_ID;
    break;
  case PjgTypes::PROJ_LATLON :
  case PjgTypes::PROJ_CYL_EQUIDIST :
    _projType = EQUIDISTANT_CYL_PROJ_ID;
    break;
  }
  
  _latin1 = _projection.getLat1();
  _latin2 = _projection.getLat2();
  _originLat = _projection.getOriginLat();
  _originLon = _projection.getOriginLon();
  _lov = _projection.getOriginLon();
  
}


bool GDS::isRegular() const
{
  return(_numVertical != 0 || _verticalOrPoints == NO_VERTICAL_POINTS);
}


void GDS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Grid Description Section (GDS):\n");
  fprintf(stream, "--------------------------\n");
  fprintf(stream, "GDS length %d\n", _nBytes);
  fprintf(stream, "Number of vertical coordinate parameters %d\n", _numVertical);
  fprintf(stream, "Vertical coord params, or numbers of pts per row %d\n", _verticalOrPoints);
  fprintf(stream, "Projection type Id %d\n", _projType);
  fprintf(stream, "Data ordering %d\n", _dataOrder);
  fprintf(stream, "Grid orientation %d\n", _gridOrientation);
  fprintf(stream, "nx %d\n", _projection.getNx());
  fprintf(stream, "ny %d\n", _projection.getNy());
  fprintf(stream, "_originLat %f\n", _originLat);
  fprintf(stream, "_originLon %f\n", _originLon);
  fprintf(stream, "_lov %f\n", _lov);
  fprintf(stream, "_latin1 %f\n", _latin1);
  fprintf(stream, "_latin2 %f\n", _latin2);

  printQuasiList(stream);
}


void GDS::print(ostream &stream) const
{
  stream << endl << endl;
  stream << "Grib Grid Description Section (GDS):" << endl;
  stream << "--------------------------" << endl;
  stream << "GDS length " << _nBytes << endl;
  stream << "Number of vertical coordinate parameters " <<  _numVertical << endl;
  stream << "Vertical coord params, or numbers of pts per row " << _verticalOrPoints << endl;
  stream << "Projection type Id " <<  _projType << endl;
  stream << "Data ordering " << _dataOrder << endl;
  stream << "Grid orientation " << _gridOrientation << endl;
  stream << "nx " <<  _projection.getNx() << endl;
  stream << "ny " <<  _projection.getNy() << endl;
  stream << "_originLat " << _originLat << endl;
  stream << "_originLon " << _originLon << endl;
  stream << "_lov " << _lov << endl;
  stream << "_latin1 " << _latin1 << endl;
  stream << "_latin2 " << _latin2 << endl;
  
  printQuasiList(stream);
}


void GDS::printQuasiList(FILE *stream) const
{
  if (isRegular())
    return;

  // print the list length 
  int listLength = 0; 
  if (_numVertical != 0) 
    listLength = _numVertical; 
  else 
    listLength = getNy(); 
  fprintf(stream, "Size of points list: %d\n", listLength);

  // print the list itself 
  for (size_t listIndex = 0; listIndex < _numPtsPerRow.size(); listIndex++) { 
    fprintf(stream, "%d ", _numPtsPerRow[listIndex]); 
  }
  fprintf(stream, "\n");
}


void GDS::printQuasiList(ostream &stream) const
{
  if (isRegular())
    return;

  // print the list length 
  int listLength = 0; 
  if (_numVertical != 0) 
    listLength = _numVertical; 
  else 
    listLength = getNy(); 
  stream << "Size of points list: " << listLength << endl;

  // print the list itself 
  for (size_t listIndex = 0; listIndex < _numPtsPerRow.size(); listIndex++) { 
    stream << _numPtsPerRow[listIndex] << " "; 
  }
  stream << endl;
}


int GDS::getNumGridPoints() const
{
  if (isRegular()) {
    // regular grid
    return getNx() * getNy();
  }

  // add up the points on each row
  int totalGridPoints = 0;
  for (vector<int>::const_iterator itr = _numPtsPerRow.begin(); itr != _numPtsPerRow.end(); itr++) {
    totalGridPoints += *itr;
  }

  return totalGridPoints;
}


int GDS::unpack(ui08 *gdsPtr)
{
   // reset private variables
   _numVertical = 0;
   _verticalOrPoints = 0;
   _projType = 0;
   _resMode = 0;
   _scanMode = 0;
   _lov = 0.0;
   _latin1 = 0.0;
   _latin2 = 0.0;
   _dataOrder = DO_XY;
   _gridOrientation = GO_SN_WE;

   _numPtsPerRow.clear();

   //
   // Length in bytes of the section
   //
   _nBytes = _upkUnsigned3( gdsPtr[0], gdsPtr[1], gdsPtr[2] );

   //
   // Check that section size is an expected value. GMC has encountered RUC files
   // with corrupt records. This check is useful in detecting bad records.
   //
   if(_nBytes < _expectedSize || _nBytes > 4096) {
     cout << "ERROR: Possible corrupt record. GDS size in bytes is " << 
       _nBytes << endl << "expected size in bytes is " << _expectedSize << endl;
     cout << "If GDS size is correct use setExpectedSize or GribRecord::setPdsExpectedSize method to pass test." 
	  << endl;
     return( GRIB_FAILURE );
   }

   // get the coordinate parameters
   _numVertical = (int)gdsPtr[3];
   _verticalOrPoints = (int)gdsPtr[4];

   //
   // Get the projection type
   //
   _projType = (int)gdsPtr[5];

   //  Different class is required if not using the latitude/longitude
   //  equidistant or Lambert Conformal projection (use EquidistantCylind).
   if (_projType == EQUIDISTANT_CYL_PROJ_ID
     || _projType == POLAR_STEREOGRAPHIC_PROJ_ID
     || _projType == GAUSSIAN_LAT_LON_PROJ_ID) {
     // do nothing -> handle this outside of this class
     return (GRIB_SUCCESS);
   }

   else if (_projType == LAMBERT_CONFORMAL_PROJ_ID) {
     // 
     // Number of points in X and Y
     //
     int nx = _upkUnsigned2(gdsPtr[6], gdsPtr[7]);
     int ny = _upkUnsigned2(gdsPtr[8], gdsPtr[9]);

     // 
     // first point latitude and longitude
     //
     double first_lat = DEGREES_SCALE_FACTOR *
       (double)_upkSigned3(gdsPtr[10], gdsPtr[11], gdsPtr[12]);
     double first_lon = DEGREES_SCALE_FACTOR *
       (double)_upkSigned3(gdsPtr[13], gdsPtr[14], gdsPtr[15]);

     // convert to east positive reference
     while(first_lon > 180.0) 
	 first_lon -= 360.0;

 
     //
     // orientation of the grid -- the east longitude value of the meridian
     // which is parallel to the y-axis along which latitude increases as the 
     // y-cooordinate increases.
     //
     _resMode = (int)gdsPtr[16];
     _lov = DEGREES_SCALE_FACTOR * (double)_upkSigned3(gdsPtr[17], gdsPtr[18], gdsPtr[19]);

     // convert to east positive reference
     while(_lov > 180)
	 _lov -= 360.0;

     //
     // dx and dy
     //
     double dx = GRID_SCALE_FACTOR *
       (double)_upkSigned3(gdsPtr[20], gdsPtr[21], gdsPtr[22]);
     double dy = GRID_SCALE_FACTOR *
       (double)_upkSigned3(gdsPtr[23], gdsPtr[24], gdsPtr[25]);

     //
     // scanning
     //
     _scanMode = (int)gdsPtr[27];

     // 
     // standard parallels
     //
     _latin1 = DEGREES_SCALE_FACTOR * (double)_upkSigned3(gdsPtr[28], gdsPtr[29], gdsPtr[30]);
     _latin2 = DEGREES_SCALE_FACTOR * (double)_upkSigned3(gdsPtr[31], gdsPtr[32], gdsPtr[33]);

#ifdef NOT_NOW
   if (((int)gdsPtr[10] & 64) == 0) {
     cout << "_firstLat is referenced from the south" << endl << flush;
   }
   if (((int)gdsPtr[13] & 64) == 0) {
     cout << "_firstLon is referenced from the west" << endl << flush;
   }
   if ( _resMode == 8) {
     cout << "spherical earth; u and v components quantities resolved relative "
       "to the defined grid." << endl << flush;
   }
   if (((int)gdsPtr[17] & 64) == 0) {
     cout << "_lov is referenced from the west" << endl << flush;
   }
   if (((int)gdsPtr[28] & 64) == 0) {
     cout << "_latin1 is referenced from the south" << endl << flush;
   }
   if (((int)gdsPtr[31] & 64) == 0) {
     cout << "_latin2 is referenced from the south" << endl << flush;
   }
#endif

     //
     // origin latitude and longitude
     //
     _originLat = _latin1;
     _originLon = _lov;
    

     //
     // data ordering
     //
     if ((_scanMode & 32) == 0) {
       _dataOrder = DO_XY;
     }
     else {
       _dataOrder = DO_YX;
     }
   
     // 
     // grid orientation
     //
     if ((_scanMode & 192) == 64) {
       _gridOrientation = GO_SN_WE;
     }
     else if ((_scanMode & 192) == 0) {
       _gridOrientation = GO_NS_WE;
     }
     else if ((_scanMode & 192) == 192) {
       _gridOrientation = GO_SN_EW;
     }
     else if ((_scanMode & 192) == 128) {
       _gridOrientation = GO_NS_EW;
     }


     // 
     // calculate min_x and min_y
     //
     double min_x, min_y;
     
     PjgCalc *calculator;
     
     if (_latin1 == _latin2)
       calculator = new PjgLc1Calc(_originLat, _originLon, _latin1);
     else
       calculator = new PjgLc2Calc(_originLat, _originLon, _latin1, _latin2);

     calculator->latlon2xy(first_lat, first_lon, min_x, min_y);
     delete(calculator);

     // Initialize the underlying projection object

     _projection.initLc2(_originLat, _originLon, _latin1, _latin2,
			 nx, ny, 1, dx, dy, 1.0,
			 min_x, min_y, 0.0);

     unpackPtsPerRow(gdsPtr);

     return( GRIB_SUCCESS );
  }
  else {
    cerr << "ERROR: Projection " << _projType << " not yet implemented\n" << endl;
    return( GRIB_FAILURE );
  }
}


int GDS::unpackPtsPerRow(ui08 *gdsPtr)
// This routine does NOT handle those situations where we have
// a list of vertical coordinate parameters.
{
  // clear any previous list
  _numPtsPerRow.clear();

  if (_numVertical != 0 || _verticalOrPoints == NO_VERTICAL_POINTS)
    return GRIB_SUCCESS;

  // retrieve the list length 
  int listLength = getNy(); 
 
  // retrieve the list itself 
  for (int listIndex = 0; listIndex < listLength; listIndex++) { 
    _numPtsPerRow.push_back(_upkSigned2( 
      gdsPtr[_verticalOrPoints + 2 * listIndex + 1], 
      gdsPtr[_verticalOrPoints + 2 * listIndex])); 
  }

  return( GRIB_SUCCESS );
}


int GDS::pack(ui08 *gdsPtr)
{
  // Length in bytes of the section

  _pkUnsigned3(_nBytes, &(gdsPtr[0]));

  // vertical coordinate parameters
  gdsPtr[3] = (ui08)_numVertical;
  gdsPtr[4] = (ui08)_verticalOrPoints;

  // Projection type
  gdsPtr[5] = (ui08)_projType;
  
  // Pack based on projection type

  if (_projType == LAMBERT_CONFORMAL_PROJ_ID)
  {
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

    // standard parallels

    _pkSigned3((int)(_latin1 / DEGREES_SCALE_FACTOR), &(gdsPtr[28]));
    _pkSigned3((int)(_latin2 / DEGREES_SCALE_FACTOR), &(gdsPtr[31]));

     /// \todo Pack the number of points per row for irregular grids.
     /// This would be something like a call to
     ///              packPtsPerRow(gdsPtr);  // does not exist
     /// Carl Drews - April 22, 2004
  }
  else if (_projType == EQUIDISTANT_CYL_PROJ_ID)
  {
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

    _pkSigned3((int)(getFirstLon() / DEGREES_SCALE_FACTOR), &(gdsPtr[13]));

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

    _pkSigned3((int)(getLastLon() / DEGREES_SCALE_FACTOR), &(gdsPtr[20]));
    
    // dx and dy

    _pkSigned2((int)(_projection.getDx() / DEGREES_SCALE_FACTOR), &(gdsPtr[23]));
    _pkSigned2((int)(_projection.getDy() / DEGREES_SCALE_FACTOR), &(gdsPtr[25]));

    // assume projection is from north pole

    gdsPtr[27] = _setScanMode();


  }
  else if (_projType == POLAR_STEREOGRAPHIC_PROJ_ID)
  {
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

  }
  else
  {
    cerr << "ERROR: Projection " << _projType << " not yet implemented\n" << endl;
    return GRIB_FAILURE;
  }

  return GRIB_SUCCESS;
}


void GDS::setRegular(int numRows, int numColumns)
// numRows is not used here because it stays constant for the Wafs grid.
// But we may need to adjust numRows in other transformations.
{
  _verticalOrPoints = NO_VERTICAL_POINTS;
  _numPtsPerRow.clear();

  // adjust the X-extent of the projection
  int nx, ny, nz;
  _projection.getGridDims(nx, ny, nz);
  _projection.setGridDims(numColumns, ny, nz);

  /// \todo We need a way to set the new delta X here, but we are missing
  /// some information.  We need the minimum and maximum longitude.
  /// Carl Drews - April 23, 2004
}


//
// little method to set scanning mode flags
//
ui08 GDS::_setScanMode()
{
  ui08 scanMode = 0;
  
  if(_dataOrder == DO_XY) {
    scanMode = 0;
  }
  else {
    scanMode = 32;
  }
  
  if(_gridOrientation == GO_SN_WE) {
    scanMode += 64;
  }
  else if(_gridOrientation == GO_NS_WE) {
    scanMode += 0;
  }
  else if(_gridOrientation == GO_SN_EW) {
    scanMode += 192;
  }
  else if(_gridOrientation == GO_NS_EW) {
    scanMode += 128;
  }
  
  return scanMode;
}

