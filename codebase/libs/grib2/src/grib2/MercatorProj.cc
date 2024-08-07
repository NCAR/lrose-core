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
// MercatorProj - Mercator  Projection
//
//
// Note:  pack and unpack routines are static
//        methods in <grib/GribSection.hh>
//////////////////////////////////////////////////

#include <iostream>

#include <grib2/MercatorProj.hh>
#include <grib2/GDS.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

const int MercatorProj::MERCATOR_SIZE = 72;

MercatorProj::MercatorProj() 
: GribProj()
{

}

MercatorProj::~MercatorProj() {

}


int MercatorProj::pack (g2_ui08 *projPtr) 
{

  projPtr[0] = (g2_ui08) _earthShape;

  projPtr[1] = (g2_ui08) _radiusScaleFactor;

  GribSection::_pkUnsigned4(_radiusScaleValue, &(projPtr[2]));

  projPtr[6] = (g2_ui08) _majorAxisScaleFactor;

  GribSection::_pkUnsigned4(_majorAxisScaleValue, &(projPtr[7]));

  projPtr[11] = (g2_ui08) _minorAxisScaleFactor;

  GribSection::_pkUnsigned4(_minorAxisScaleValue, &(projPtr[12]));

  GribSection::_pkUnsigned4(_ni, &(projPtr[16]));

  GribSection::_pkUnsigned4(_nj, &(projPtr[20]));

  GribSection::_pkSigned4((int)(_la1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[24]));

  GribSection::_pkSigned4((int)(_lo1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[28]));

  projPtr[32] = (g2_ui08) _resolutionFlag;

  GribSection::_pkSigned4((int)(_lad / GDS::DEGREES_SCALE_FACTOR), &(projPtr[33]));

  GribSection::_pkSigned4((int)(_la2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[37]));

  GribSection::_pkSigned4((int)(_lo2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[41]));

  projPtr[45] = (g2_ui08) _scanModeFlag;

  GribSection::_pkSigned4((int)(_lov / GDS::DEGREES_SCALE_FACTOR), &(projPtr[46]));

  GribSection::_pkUnsigned4((int)(_di / GDS::DEGREES_SCALE_FACTOR), &(projPtr[50]));

  GribSection::_pkUnsigned4((int)(_dj / GDS::DEGREES_SCALE_FACTOR), &(projPtr[54]));

  return( GRIB_SUCCESS );
}

int MercatorProj::unpack (g2_ui08 *projPtr) 
{

  // Shape of the earth (see Code Table 3.2)
  _earthShape = (g2_si32) projPtr[0]; 

  // Scale factor of radius of spherical earth
  _radiusScaleFactor = (g2_si32) projPtr[1]; 

  //Scaled value of radius of spherical earth
  _radiusScaleValue 
            = GribSection::_upkUnsigned4 (projPtr[2], projPtr[3], projPtr[4], projPtr[5]);

  // Scale factor of major axis of oblate spheroid earth
  _majorAxisScaleFactor = (g2_si32) projPtr[6]; 

  // Scaled value of major axis of oblate spheroid earth
  _majorAxisScaleValue =
           GribSection::_upkUnsigned4 (projPtr[7], projPtr[8], projPtr[9], projPtr[10]);

  // Scale factor of minor axis of oblate spheroid earth
  _minorAxisScaleFactor = (g2_si32) projPtr[11]; 

  //Scaled value of minor axis of oblate spheroid earth
  _minorAxisScaleValue = 
           GribSection::_upkUnsigned4 (projPtr[12], projPtr[13], projPtr[14], projPtr[15]);

  // number of points along a parallel (undefined - all bits set to 1)
  _ni = GribSection::_upkUnsigned4 (projPtr[16], projPtr[17], projPtr[18], projPtr[19]);

  // number of points along longitude meridian (undefined - all bits set to 1)
  _nj = GribSection::_upkUnsigned4 (projPtr[20], projPtr[21], projPtr[22], projPtr[23]);

  // Latitude of first grid point (leftmost bit set for south latitude)
  _la1 = GDS::DEGREES_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkSigned4(projPtr[24], projPtr[25], projPtr[26], projPtr[27]);

  // Longitude of first grid point (leftmost bit set for west longitude)
  _lo1 = GDS::DEGREES_SCALE_FACTOR *
           (g2_fl32) GribSection::_upkSigned4(projPtr[28], projPtr[29], projPtr[30], projPtr[31]);

  // Resolution and component flags
  _resolutionFlag = projPtr[32];

  // Latitude where Dx and Dy are specified (leftmost bit set for south latitude
  _lad = GDS::DEGREES_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkSigned4(projPtr[33], projPtr[34], projPtr[35],  projPtr[36]);

  // Latitude of last grid point (leftmost bit set for south latitude)
  _la2 = GDS::DEGREES_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkSigned4(projPtr[37], projPtr[38], projPtr[39],  projPtr[40]);

  // Longitude of last grid point (leftmost bit set for west longitude)
  _lo2 = GDS::DEGREES_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkSigned4(projPtr[41], projPtr[42], projPtr[43],  projPtr[44]);

  // Scanning mode flags
  _scanModeFlag = projPtr[45];

  // orientation of the grid
  _lov = GDS::DEGREES_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkSigned4(projPtr[46], projPtr[47], projPtr[48],  projPtr[49]);

  // Longitudinal Direction Increment  (undefined - all bits set to 1)
  _di = GDS::GRID_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkUnsigned4(projPtr[50], projPtr[51], projPtr[52], projPtr[53]);

  // Latitudinal Direction Increment (undefined - all bits set to 1)
  _dj = GDS::GRID_SCALE_FACTOR * 
           (g2_fl32) GribSection::_upkUnsigned4(projPtr[54], projPtr[55], projPtr[56], projPtr[57]);

  if(_ni == GribSection::U4MISSING || _nj == GribSection::U4MISSING ||
     _di == GribSection::S4MISSING || _dj == GribSection::S4MISSING)
  {
     cerr << "ERROR: LatLonProj::unpack()" << endl;
     cerr << "Quasi-regular Mercator grid is unimplemented" << endl;
     return (GRIB_FAILURE);
  }

  return( GRIB_SUCCESS );
}


void MercatorProj::print(FILE *stream) const
{
  fprintf(stream, "Mercator Projection:\n");

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
  fprintf(stream, "Number of points along latitude circle %d\n", _ni);
  fprintf(stream, "Number of points along longitude meridian %d\n", _nj);
  fprintf(stream, "Latitude of first grid point %f\n", _la1);
  fprintf(stream, "Longitude of first grid point %f\n", _lo1);
  fprintf(stream, "Latitude where Dx and Dy are specified (lad) %f\n", _lad);
  fprintf(stream, "Latitude of last grid point %f\n", _la2);
  fprintf(stream, "Longitude of last grid point %f\n", _lo2);
  fprintf(stream, "Orientation of the grid (lov) %f\n", _lov);
  fprintf(stream, "Longitudinal Direction Increment %f\n", _di);
  fprintf(stream, "Latitudinal Direction Increment %f\n", _dj);

  fprintf(stream, "Resolution flags %d\n", _resolutionFlag);
  if ((_resolutionFlag & 32) == 32)
    fprintf(stream, "    i direction increments not given\n");
  if ((_resolutionFlag & 16) == 16)
    fprintf(stream, "    j direction increments not given\n");
  if ((_resolutionFlag & 8) == 8) {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to easterly\n");
    fprintf(stream, "     and northerly directions\n");
  }

  fprintf(stream, "Scanning mode flags %d\n", _scanModeFlag);
  if ((_scanModeFlag & 128) == 128) {
    fprintf(stream, "    !Points of first row or column scan in the -i (-x) direction\n");
  }
  if ((_scanModeFlag & 64) == 0) {
    fprintf(stream, "    !Points of first row or column scan in the -j (-y) direction\n");
  }
  if ((_scanModeFlag & 32) == 32) {
    fprintf(stream, "    !Adjacent points in j (y) direction are consecutive\n");
  }
  if ((_scanModeFlag & 16) == 16) {
    fprintf(stream, "    !Adjacent rows scans in the opposite direction\n");
  }
  if ((_scanModeFlag & 8) == 8) {
    fprintf(stream, "    !Points within odd rows are offset by Di/2 in i(x) direction\n");
  }
  if ((_scanModeFlag & 4) == 4) {
    fprintf(stream, "    !Points within even rows are offset by Di/2 in i(x) direction\n");
  }
  if ((_scanModeFlag & 2) == 2) {
    fprintf(stream, "    !Points are offset by Di/2 in j(y) direction\n");
  }
  if ((_scanModeFlag & 1) == 1) {
    fprintf(stream, "    Rows have Ni grid points if points are not offset in i direction\n");
    fprintf(stream, "    Rows have Ni-1 grid points if points are offset by Di/2 in i direction\n");
    fprintf(stream, "    Columns have Nj grid points if points are not offset in j direction\n");
    fprintf(stream, "    Columns have Nj-1 grid points if points are offset by Dj/2 in j direction\n");
  }
}

} // namespace Grib2

