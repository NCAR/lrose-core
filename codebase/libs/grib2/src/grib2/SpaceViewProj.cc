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
// SpaceViewProj - Space View Perspective or Orthographic
//              projection definition
//
// Note:  pack and unpack routines are static
//        methods in <grib/GribSection.hh>
//////////////////////////////////////////////////

#include <iostream>

#include <grib2/SpaceViewProj.hh>
#include <grib2/GDS.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

const int SpaceViewProj::LAT_LON_SIZE = 72;

SpaceViewProj::SpaceViewProj() 
: GribProj()
{

}


SpaceViewProj::~SpaceViewProj() {

}


int SpaceViewProj::pack (ui08 *projPtr) 
{

  projPtr[0] = (ui08) _earthShape;

  projPtr[1] = (ui08) _radiusScaleFactor;

  GribSection::_pkUnsigned4(_radiusScaleValue, &(projPtr[2]));

  projPtr[6] = (ui08) _majorAxisScaleFactor;

  GribSection::_pkUnsigned4(_majorAxisScaleValue, &(projPtr[7]));

  projPtr[11] = (ui08) _minorAxisScaleFactor;

  GribSection::_pkUnsigned4(_minorAxisScaleValue, &(projPtr[12]));

  GribSection::_pkUnsigned4(_nx, &(projPtr[16]));

  GribSection::_pkUnsigned4(_ny, &(projPtr[20]));

  GribSection::_pkSigned4((int)(_lap / GDS::GRID_SCALE_FACTOR), &(projPtr[24]));

  GribSection::_pkSigned4((int)(_lop / GDS::GRID_SCALE_FACTOR), &(projPtr[28]));

  projPtr[32] = (ui08) _resolutionFlag;

  GribSection::_pkUnsigned4(_dx, &(projPtr[33]));

  GribSection::_pkUnsigned4(_dy, &(projPtr[37]));

  GribSection::_pkUnsigned4((int)(_xp / 0.001), &(projPtr[41]));

  GribSection::_pkUnsigned4((int)(_yp / 0.001), &(projPtr[45]));

  projPtr[49] = (ui08) _scanModeFlag;

  GribSection::_pkUnsigned4((int)(_lov / GDS::GRID_SCALE_FACTOR), &(projPtr[50]));

  GribSection::_pkUnsigned4((int)(_nr / GDS::GRID_SCALE_FACTOR), &(projPtr[54]));

  GribSection::_pkUnsigned4(_xo, &(projPtr[58]));

  GribSection::_pkUnsigned4(_yo, &(projPtr[62]));

  return( GRIB_SUCCESS );
}

int SpaceViewProj::unpack (ui08 *projPtr) 
{

  // Shape of the earth (see Code Table 3.2)
  _earthShape = (si32) projPtr[0]; 

  // Scale factor of radius of spherical earth
  _radiusScaleFactor = (si32) projPtr[1]; 

  // Scaled value of radius of spherical earth
  _radiusScaleValue 
            = GribSection::_upkUnsigned4 (projPtr[2], projPtr[3], projPtr[4], projPtr[5]);

  // Scale factor of major axis of oblate spheroid earth
  _majorAxisScaleFactor = (si32) projPtr[6]; 

  // Scaled value of major axis of oblate spheroid earth
  _majorAxisScaleValue =
           GribSection::_upkUnsigned4 (projPtr[7], projPtr[8], projPtr[9], projPtr[10]);

  // Scale factor of minor axis of oblate spheroid earth
  _minorAxisScaleFactor = (si32) projPtr[11]; 

  //Scaled value of minor axis of oblate spheroid earth
  _minorAxisScaleValue = 
           GribSection::_upkSigned4 (projPtr[12], projPtr[13], projPtr[14], projPtr[15]);

  // Number of points along x-axis (columns)
  _nx = GribSection::_upkUnsigned4 (projPtr[16], projPtr[17], projPtr[18], projPtr[19]);

  // Number of points along y-axis (rows or lines)
  _ny = GribSection::_upkUnsigned4 (projPtr[20], projPtr[21], projPtr[22], projPtr[23]);

  // Latitude of sub-satellite point
  _lap = GDS::GRID_SCALE_FACTOR * (fl32) GribSection::_upkSigned4(projPtr[24], projPtr[25], projPtr[26], projPtr[27]);

  // Longitude of sub-satellite point
  _lop = GDS::GRID_SCALE_FACTOR * (fl32) GribSection::_upkSigned4(projPtr[28], projPtr[29], projPtr[30], projPtr[31]);

  // Resolution and component flags
  _resolutionFlag = projPtr[32];

  // Apparent diameter of Earth in grid lengths, in x-direction
  _dx = (fl32) GribSection::_upkUnsigned4(projPtr[33], projPtr[34], projPtr[35],  projPtr[36]);

  // Apparent diameter of Earth in grid lengths, in y-direction 
  _dy = (fl32) GribSection::_upkUnsigned4(projPtr[37], projPtr[38], projPtr[39],  projPtr[40]);

  // X-coordinate of sub-satellite point (in units of 10-3 grid length expressed as an integer)
  _xp = 0.001 * (fl32) GribSection::_upkUnsigned4(projPtr[41], projPtr[42], projPtr[43],  projPtr[44]);

  // Y-coordinate of sub-satellite point (in units of 10-3 grid length expressed as an integer)
  _yp = 0.001 * (fl32) GribSection::_upkUnsigned4(projPtr[45], projPtr[46], projPtr[47],  projPtr[48]);

  // Scanning mode flags
  _scanModeFlag = projPtr[49];

  // Orientation of the grid
  // the angle between the increasing y-axis and the meridian of the sub-satellite 
  // point in the direction of increasing latitude
  _lov = GDS::GRID_SCALE_FACTOR * (fl32) GribSection::_upkUnsigned4(projPtr[50], projPtr[51], projPtr[52], projPtr[53]);

  // Altitude of the camera from the Earth's centre, measured in units of the Earth's (equatorial)
  // radius multiplied by a scale factor of 10^6  (missing = all bits set to 1 = infinite distance)
  // The apparent angular size of the Earth will be given by 2 × arcsin ((10^6 )/Nr). 
  _nr = GDS::GRID_SCALE_FACTOR * (fl32) GribSection::_upkUnsigned4(projPtr[54], projPtr[55], projPtr[56], projPtr[57]);

  // X-coordinate of origin of sector image
  _xo = (fl32) GribSection::_upkUnsigned4(projPtr[58], projPtr[59], projPtr[60], projPtr[61]);

  // Y-coordinate of origin of sector i
  _yo = (fl32) GribSection::_upkUnsigned4(projPtr[62], projPtr[63], projPtr[64], projPtr[65]);


  return( GRIB_SUCCESS );
}


void SpaceViewProj::print(FILE *stream) const
{
  fprintf(stream, "Space View Perspective or Orthographic projection:\n");

  switch (_earthShape) {
    case 0:
      fprintf(stream, "Earth assumed spherical with radius = 6367.4700 km\n");
      break;
    case 1:
      fprintf(stream, "Earth assumed spherical with radius specified by data producer\n");
      break;
    case 2:
      fprintf(stream, "Earth assumed oblate spheroid with size as determined by IAU in 1965\n");
      fprintf(stream, "(major axis = 6378.160 km, minor axis = 6356.775 km, f = 1/297.0)\n");
	break;
    case 3:
      fprintf(stream, "Earth assumed oblate spheroid with major and minor axes specified by data producer\n");
      break;
    case 4:
      fprintf(stream, "Earth assumed oblate spheroid with size as determined by IAG-GRS80 model\n");
      fprintf(stream, "(major axis = 6378.1370 km, minor axis = 6356.752314 km, f = 1/298.257222101)\n");
	break;
    case 5:
      fprintf(stream, "Earth assumed represented by WGS84 (as used by ICAO since 1998)(Uses IAG-GRS80 as a basis)\n");
      break;
    case 6:
      fprintf(stream, "Earth assumed spherical with radius = 6371.2290 km\n");
      break;
    case 255:
      fprintf(stream, "Earth Shape flag Missing\n");
      break;
    default:
      if (_earthShape >= 7 && _earthShape <= 191)
         fprintf(stream, "Earth shape in reserved area, value found is %d\n", _earthShape);
      else if (_earthShape >= 192 && _earthShape <= 254)
         fprintf(stream, "Earth shape in local reserved area, value found is %d\n", _earthShape);

  }
  fprintf(stream, "Scale factor of radius of spherical earth %d\n", _radiusScaleFactor);
  fprintf(stream, "Scaled value of radius of spherical earth %d\n", _radiusScaleValue);
  fprintf(stream, "Scale factor of major axis of oblate spheroid earth %d\n", _majorAxisScaleFactor);
  fprintf(stream, "Scaled value of major axis of oblate spheroid earth %d\n", _majorAxisScaleValue);
  fprintf(stream, "Scale factor of minor axis of oblate spheroid earth %d\n", _minorAxisScaleFactor);
  fprintf(stream, "Scaled value of minor axis of oblate spheroid earth %d\n", _minorAxisScaleValue);
  fprintf(stream, "Number of points along x-axis (columns) %d\n", _nx);
  fprintf(stream, "Number of points along y-axis (rows or lines) %d\n", _ny);
  fprintf(stream, "Latitude of sub-satellite point %f\n", _lap);
  fprintf(stream, "Longitude of sub-satellite point %f\n", _lop);
  fprintf(stream, "Resolution flag byte %d\n", _resolutionFlag);
  if ((_resolutionFlag & 32) == 32)
    fprintf(stream, "    i direction increments given\n");
  else
    fprintf(stream, "    i direction increments not given\n");

  if ((_resolutionFlag & 16) == 16)
    fprintf(stream, "    j direction increments given\n");
  else
    fprintf(stream, "    j direction increments not given\n");

  if ((_resolutionFlag & 8) == 8) {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to the defined\n");
    fprintf(stream, "    grid in the direction of increasing x and y (or i and j) coordinates respectively\n");
  }
  else {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to easterly\n");
    fprintf(stream, "     and northerly directions\n");
  }

  fprintf(stream, "Apparent diameter of Earth in grid lengths, in x-direction %d\n", _dx);
  fprintf(stream, "Apparent diameter of Earth in grid lengths, in y-direction %d\n", _dy);
  fprintf(stream, "X-coordinate of sub-satellite point %d\n", _xp);
  fprintf(stream, "Y-coordinate of sub-satellite point %d\n", _yp);
  fprintf(stream, "Scanning mode flags %d\n", _scanModeFlag);

  if ((_scanModeFlag & 16) == 0) {
    fprintf(stream, "    All rows scan in the same direction\n");
  }
  else {
    fprintf(stream, "    Adjacent rows scans in the opposite direction\n");
  }

  if ((_scanModeFlag & 32) == 0) {
    fprintf(stream, "    Adjacent points in i (x) direction are consecutive\n");
  }
  else {
    fprintf(stream, "    Adjacent points in j (y) direction are consecutive\n");
  }

  if ((_scanModeFlag & 64) == 0) {
    fprintf(stream, "    Points of first row or column scan in the -j (-y) direction\n");
  }
  else {
    fprintf(stream, "    Points of first row or column scan in the +j (+y) direction\n");
  }

  if ((_scanModeFlag & 128) == 0) {
    fprintf(stream, "    Points of first row or column scan in the +i (+x) direction\n");
  }
  else {
    fprintf(stream, "    Points of first row or column scan in the -i (-x) direction\n");
  }

  fprintf(stream, "Orientation of the grid  %f\n", _lov);
  fprintf(stream, "Altitude of the camera from the Earth's centre %f\n", _nr);
  fprintf(stream, "X-coordinate of origin of sector image %d\n", _xo);
  fprintf(stream, "Y-coordinate of origin of sector i %d\n", _yo);

}

} // namespace Grib2

