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
// PolarStereoProj - Polar Stereographic Projection
//
//
// Note:  pack and unpack routines are static
//        methods in <grib/GribSection.hh>
//////////////////////////////////////////////////

#include <grib2/PolarStereoProj.hh>
#include <grib2/GDS.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

const int PolarStereoProj::POLAR_STEREO_SIZE = 72;

PolarStereoProj::PolarStereoProj() 
: GribProj()
{

}

PolarStereoProj::~PolarStereoProj() {

}


int PolarStereoProj::pack (ui08 *projPtr) 
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

  GribSection::_pkSigned4((int)(_la1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[24]));

  GribSection::_pkSigned4((int)(_lo1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[28]));

  projPtr[32] = (ui08) _resolutionFlag;

  GribSection::_pkSigned4((int)(_lad / GDS::DEGREES_SCALE_FACTOR), &(projPtr[33]));

  GribSection::_pkSigned4((int)(_lov / GDS::DEGREES_SCALE_FACTOR), &(projPtr[37]));

  GribSection::_pkUnsigned4((int)(_dx / GDS::DEGREES_SCALE_FACTOR), &(projPtr[41]));

  GribSection::_pkUnsigned4((int)(_dy / GDS::DEGREES_SCALE_FACTOR), &(projPtr[45]));

  projPtr[49] = (ui08) _projCtrFlag;

  projPtr[50] = (ui08) _scanModeFlag;

  return( GRIB_SUCCESS );
}

int PolarStereoProj::unpack (ui08 *projPtr) 
{

  // Shape of the earth (see Code Table 3.2)
  _earthShape = (si32) projPtr[0]; 

  // Scale factor of radius of spherical earth
  _radiusScaleFactor = (si32) projPtr[1]; 

  //Scaled value of radius of spherical earth
  _radiusScaleValue 
            = GribSection::_upkSigned4 (projPtr[2], projPtr[3], projPtr[4], projPtr[5]);

  // Scale factor of major axis of oblate spheroid earth
  _majorAxisScaleFactor = (si32) projPtr[6]; 

  // Scaled value of major axis of oblate spheroid earth
  _majorAxisScaleValue =
           GribSection::_upkSigned4 (projPtr[7], projPtr[8], projPtr[9], projPtr[10]);

  // Scale factor of minor axis of oblate spheroid earth
  _minorAxisScaleFactor = (si32) projPtr[11]; 

  //Scaled value of minor axis of oblate spheroid earth
  _minorAxisScaleValue = 
           GribSection::_upkSigned4 (projPtr[12], projPtr[13], projPtr[14], projPtr[15]);

  // number of points along X-axis
  _nx = GribSection::_upkUnsigned4 (projPtr[16], projPtr[17], projPtr[18], projPtr[19]);

  // number of points along Y-axis
  _ny = GribSection::_upkUnsigned4 (projPtr[20], projPtr[21], projPtr[22], projPtr[23]);

  // Latitude of first grid point (leftmost bit set for south latitude)
  _la1 = GDS::DEGREES_SCALE_FACTOR * 
           (fl32) GribSection::_upkSigned4(projPtr[24], projPtr[25], projPtr[26], projPtr[27]);

  // Longitude of first grid point (leftmost bit set for west longitude)
  _lo1 = GDS::DEGREES_SCALE_FACTOR *
           (fl32) GribSection::_upkSigned4(projPtr[28], projPtr[29], projPtr[30], projPtr[31]);

  // Resolution and component flags
  _resolutionFlag = projPtr[32];

  // Latitude where Dx and Dy are specified (leftmost bit set for south latitude
  _lad = GDS::DEGREES_SCALE_FACTOR * 
           (fl32) GribSection::_upkSigned4(projPtr[33], projPtr[34], projPtr[35],  projPtr[36]);

  // orientation of the grid
  _lov = GDS::DEGREES_SCALE_FACTOR * 
           (fl32) GribSection::_upkSigned4(projPtr[37], projPtr[38], projPtr[39],  projPtr[40]);

  // X-axis Direction Increment
  _dx = GDS::GRID_SCALE_FACTOR * 
           (fl32) GribSection::_upkUnsigned4(projPtr[41], projPtr[42], projPtr[43], projPtr[44]);

  // Y-axis Direction Increment
  _dy = GDS::GRID_SCALE_FACTOR * 
           (fl32) GribSection::_upkUnsigned4(projPtr[45], projPtr[46], projPtr[47], projPtr[48]);

  // Projection Centre Flag
  _projCtrFlag = projPtr[49];

  // Scanning mode flags
  _scanModeFlag = projPtr[50];


  return( GRIB_SUCCESS );
}


void PolarStereoProj::print(FILE *stream) const
{
  fprintf(stream, "Polar Stereographic Projection:\n");

  switch (_earthShape) {
    case 0:
      fprintf(stream, "Earth assumed spherical with radius = 6367.47 km\n");
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
  fprintf(stream, "Number of points along X-Axis %d\n", _nx);
  fprintf(stream, "Number of points along Y-Axis %d\n", _ny);
  fprintf(stream, "Latitude of first grid point %f\n", _la1);
  fprintf(stream, "Longitude of first grid point %f\n", _lo1);
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

  fprintf(stream, "Latitude where Dx and Dy are specified (lad) %f\n", _lad);
  fprintf(stream, "Orientation of the grid (lov) %f\n", _lov);
  fprintf(stream, "X-Axis Direction Increment %f\n", _dx);
  fprintf(stream, "Y-Axis Direction Increment %f\n", _dy);

  fprintf(stream, "Projection Centre flag %d\n", _projCtrFlag);

  if ((_projCtrFlag & 128) == 0) {
    fprintf(stream, "    North Pole is on the projection plane\n");
  }
  else {
    fprintf(stream, "    South Pole is on the projection plane\n");
  }

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

}

} // namespace Grib2

