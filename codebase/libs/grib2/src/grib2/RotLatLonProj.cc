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
// RotLatLonProj - Rotated Latitude/Longitude projection
//              definition - also called Equidistant 
//              Cylindrical grid or or Plate Carree 
//              projection 
//
// Note:  pack and unpack routines are static
//        methods in <grib/GribSection.hh>
//////////////////////////////////////////////////

#include <iostream>

#include <grib2/RotLatLonProj.hh>
#include <grib2/GDS.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

const int RotLatLonProj::LAT_LON_SIZE = 72;

RotLatLonProj::RotLatLonProj() 
: GribProj()
{

}


RotLatLonProj::~RotLatLonProj() {

}


int RotLatLonProj::pack (ui08 *projPtr) 
{

  projPtr[0] = (ui08) _earthShape;

  projPtr[1] = (ui08) _radiusScaleFactor;

  GribSection::_pkUnsigned4(_radiusScaleValue, &(projPtr[2]));

  projPtr[6] = (ui08) _majorAxisScaleFactor;

  GribSection::_pkUnsigned4(_majorAxisScaleValue, &(projPtr[7]));

  projPtr[11] = (ui08) _minorAxisScaleFactor;

  GribSection::_pkUnsigned4(_minorAxisScaleValue, &(projPtr[12]));

  GribSection::_pkUnsigned4(_ni, &(projPtr[16]));

  GribSection::_pkUnsigned4(_nj, &(projPtr[20]));

  GribSection::_pkUnsigned4((int)(_basicAngleProdDomain / GDS::DEGREES_SCALE_FACTOR), &(projPtr[24]));

  GribSection::_pkUnsigned4((int)(_basicAngleSubdivisions / GDS::DEGREES_SCALE_FACTOR), &(projPtr[28]));

  GribSection::_pkSigned4((int)(_la1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[32]));

  GribSection::_pkSigned4((int)(_lo1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[36]));

  projPtr[40] = (ui08) _resolutionFlag;

  GribSection::_pkSigned4((int)(_la2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[41]));

  GribSection::_pkSigned4((int)(_lo2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[45]));

  GribSection::_pkUnsigned4((int)(_di / GDS::DEGREES_SCALE_FACTOR), &(projPtr[49]));

  GribSection::_pkUnsigned4((int)(_dj / GDS::DEGREES_SCALE_FACTOR), &(projPtr[53]));

  projPtr[57] = (ui08) _scanModeFlag;

  GribSection::_pkSigned4((int)(_laSpole / GDS::DEGREES_SCALE_FACTOR), &(projPtr[58]));

  GribSection::_pkSigned4((int)(_loSpole / GDS::DEGREES_SCALE_FACTOR), &(projPtr[62]));

  GribSection::_pkUnsigned4((int)(_rotation / GDS::DEGREES_SCALE_FACTOR), &(projPtr[66]));

  return( GRIB_SUCCESS );
}

int RotLatLonProj::unpack (ui08 *projPtr) 
{

  // Shape of the earth (see Code Table 3.2)
  _earthShape = (si32) projPtr[0]; 

  // Scale factor of radius of spherical earth
  _radiusScaleFactor = (si32) projPtr[1]; 

  //Scaled value of radius of spherical earth
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
           GribSection::_upkUnsigned4 (projPtr[12], projPtr[13], projPtr[14], projPtr[15]);

  // number of points along a parallel (undefined - all bits set to 1)
  _ni = GribSection::_upkUnsigned4 (projPtr[16], projPtr[17], projPtr[18], projPtr[19]);

  // number of points along  longitude meridian (undefined - all bits set to 1)
  _nj = GribSection::_upkUnsigned4 (projPtr[20], projPtr[21], projPtr[22], projPtr[23]);

  // Basic angle of the initial production domain (see Note 1)
  _basicAngleProdDomain = GDS::DEGREES_SCALE_FACTOR *
           (fl32) GribSection::_upkUnsigned4 (projPtr[24], projPtr[25], projPtr[26], projPtr[27]);

  // Subdivisions of basic angle used to define extreme longitudes and latitudes, 
  // and direction increments(see Note 1)
  _basicAngleSubdivisions = GDS::DEGREES_SCALE_FACTOR *
           (fl32) GribSection::_upkUnsigned4 (projPtr[28], projPtr[29], projPtr[30], projPtr[31]);

  float units = GDS::DEGREES_SCALE_FACTOR;
  if(_basicAngleProdDomain != 0.0)
    units = _basicAngleProdDomain / _basicAngleSubdivisions;

  // Latitude of first grid point (leftmost bit set for south latitude)
  _la1 = units * (fl32) GribSection::_upkSigned4(projPtr[32], projPtr[33], projPtr[34], projPtr[35]);

  // Longitude of first grid point (leftmost bit set for west longitude)
  _lo1 = units * (fl32) GribSection::_upkSigned4(projPtr[36], projPtr[37], projPtr[38], projPtr[39]);

  // Resolution and component flags
  _resolutionFlag = projPtr[40];

  // Latitude of last grid point (leftmost bit set for south latitude
  _la2 = units * (fl32) GribSection::_upkSigned4(projPtr[41], projPtr[42], projPtr[43],  projPtr[44]);

  // Longitude of last grid point (leftmost bit set for west longitude
  _lo2 = units * (fl32) GribSection::_upkSigned4(projPtr[45], projPtr[46], projPtr[47],  projPtr[48]);

  // Longitudinal Direction Increment  (undefined - all bits set to 1)
  _di = units * (fl32) GribSection::_upkUnsigned4(projPtr[49], projPtr[50], projPtr[51], projPtr[52]);

  // Latitudinal Direction Increment (undefined - all bits set to 1)
  _dj = units * (fl32) GribSection::_upkUnsigned4(projPtr[53], projPtr[54], projPtr[55], projPtr[56]);

  // Scanning mode flags
  _scanModeFlag = projPtr[57];

  if(_ni == GribSection::U4MISSING || _nj == GribSection::U4MISSING ||
     _di == GribSection::S4MISSING || _dj == GribSection::S4MISSING)
  {
     cerr << "ERROR: RotLatLonProj::unpack()" << endl;
     cerr << "Quasi-regular Lat/Lon grid is unimplemented" << endl;
     return (GRIB_FAILURE);
  }

  // Latitude of southern pole of projection
  _laSpole = units * (fl32) GribSection::_upkSigned4(projPtr[58], projPtr[59], projPtr[60],  projPtr[61]);

  // Longitude of southern pole of projection
  _loSpole = units * (fl32) GribSection::_upkSigned4(projPtr[62], projPtr[63], projPtr[64],  projPtr[65]);

  // Angle of roation of projection
  _rotation = units * (fl32) GribSection::_upkUnsigned4(projPtr[66], projPtr[67], projPtr[68], projPtr[69]);

  return( GRIB_SUCCESS );
}


void RotLatLonProj::print(FILE *stream) const
{
  fprintf(stream, "Rotated Latitude/longitude projection:\n");

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
  fprintf(stream, "Number of points along latitude circle %d\n", _ni);
  fprintf(stream, "Number of points along longitude meridian %d\n", _nj);
  fprintf(stream, "Basic angle of the initial production domain %f\n", _basicAngleProdDomain);
  fprintf(stream, "Subdivisions of basic angle used to define extreme longitudes and latitudes, %f\n",_basicAngleSubdivisions);
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

  fprintf(stream, "Latitude of last grid point %f\n", _la2);
  fprintf(stream, "Longitude of last grid point %f\n", _lo2);
  fprintf(stream, "Longitudinal Direction Increment %f\n", _di);
  fprintf(stream, "Latitudinal Direction Increment %f\n", _dj);
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
  fprintf(stream, "Latitude of southern pole of projection %f\n", _laSpole);
  fprintf(stream, "Longitude of southern pole of projection %f\n", _loSpole);
  fprintf(stream, "Angle of rotation of projection %f\n", _rotation);
}

} // namespace Grib2

