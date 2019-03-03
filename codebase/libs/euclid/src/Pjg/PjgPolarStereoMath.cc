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
//////////////////////////////////////////////////////////
// PjgPolarStereoMath.cc
//
// Low-level math for Stereographic projection in the
// polar aspect
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
//////////////////////////////////////////////////////////

#include <euclid/PjgMath.hh>
#include <cmath>
using namespace std;

#define TINY_ANGLE 1.e-4
#define TINY_DBL 1.e-10

////////////////////////
// Constructor

PjgPolarStereoMath::PjgPolarStereoMath(double tangent_lon, 
                                       bool pole_is_north /* = true */,
                                       double central_scale /* = 1.0*/) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_POLAR_STEREO;
  
  setTangentLon(tangent_lon);
  setPole(pole_is_north);
  setCentralScale(central_scale);
}

////////////////////////////////////////////////
/// set methods

void PjgPolarStereoMath::setTangentLon(const double tangent_lon)
{
  _origin_lon = tangent_lon; 
  _tangent_lon = tangent_lon;
  _tangent_lon_rad = _tangent_lon * Pjg::Deg2Rad;
  _offset_lon = _origin_lon;
}

void PjgPolarStereoMath::setPole(const bool pole_is_north)
{
  _pole_is_north = pole_is_north;
  
  if(_pole_is_north) {
    _origin_lat = 90.0;
    _tangent_lat = 90.0;
    _sin_tangent_lat = 1.0;
  } else {
    _origin_lat = -90.0;
    _tangent_lat = -90.0;
    _sin_tangent_lat = -1.0;
  }
  
  _offset_lat = _origin_lat;
}

void PjgPolarStereoMath::setCentralScale(const double central_scale)
{
  _central_scale = central_scale;
  if( _central_scale == 0.0) {
    _central_scale = 1.0;
  }
}

///////////////
// print object

void PjgPolarStereoMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg) : " << _origin_lon << endl;
  out << "  origin_lat (deg) : " << _origin_lat << endl;
  out << "  tangent_lon (deg): " << _tangent_lon << endl;
  out << "  pole_is_north    : " << _pole_is_north << endl;
  out << "  central_scale    : " << _central_scale << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgPolarStereoMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    tangent_lon_rad: " << _tangent_lon_rad << endl;
  out << "    sin_tangent_lat: " << _sin_tangent_lat << endl;
  out << "    false_easting  : " << _false_easting << endl;
  out << "    false_northing : " << _false_northing << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgPolarStereoMath::latlon2xy(double lat, double lon,
                                   double &x, double &y,
                                   double z /* = -9999 */) const
  
{

  double lat_rad = lat * Pjg::Deg2Rad;
  double lon_rad = lon * Pjg::Deg2Rad;

  /* degenerate cases - Transform a close point */
  if (fabs(lat_rad + M_PI_2) < TINY_ANGLE) {
    lat_rad += TINY_ANGLE;
  }
  if(fabs(lon_rad - _tangent_lon_rad - M_PI) < TINY_ANGLE) {
    lon_rad -= TINY_ANGLE;
  } 
  if(fabs(lon_rad - _tangent_lon_rad + M_PI) < TINY_ANGLE) {
    lon_rad += TINY_ANGLE;
  }
  
  double sin_delta_lon, cos_delta_lon;
  EG_sincos(lon_rad - _tangent_lon_rad, &sin_delta_lon, &cos_delta_lon);

  double mult = 2.0 * Pjg::EradKm * _central_scale;
  double xx, yy;
  if (_pole_is_north) {
    double tanval = tan(M_PI_4 - lat_rad/2.0);
    xx = mult * tanval * sin_delta_lon;
    yy = -1.0 * mult * tanval * cos_delta_lon;
  } else {
    double tanval = tan(M_PI_4 + lat_rad/2.0);
    xx = mult * tanval * sin_delta_lon;
    yy = mult * tanval * cos_delta_lon;
  }


  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgPolarStereoMath::xy2latlon(double x, double y,
                                   double &lat, double &lon,
                                   double z /* = -9999 */) const
  
{

  /* first translate into coord system with origin at tangent point */

  x -= _false_easting;
  y -= _false_northing;
  
  double rho = hypot(x, y);
  double c = 2.0 * atan2( rho, 2.0 * Pjg::EradKm * _central_scale);	
  double cosc = cos(c);
  
  double phi;
  if (fabs(rho) < TINY_DBL) {
    if (_pole_is_north) {
      phi = M_PI_2;
    } else {
      phi = -M_PI_2;
    }
  } else {
    phi = asin(cosc * _sin_tangent_lat );
  }
  
  lat = phi * Pjg::Rad2Deg;
  
  double lamda;
  if ((fabs(x) < TINY_DBL) && (fabs(y) < TINY_DBL)) {
    lamda = _tangent_lon_rad;
  } else {
    if (_pole_is_north) {
      lamda = _tangent_lon_rad + atan2(x, -y);
    } else {
      lamda = _tangent_lon_rad + atan2(x, y);
    }
  }
  
  lon = conditionRange180(lamda * Pjg::Rad2Deg);
  conditionLon2Origin(lon);

}
     
///////////////////////////////////////////////////////////
// compute the central scale given a standard parallel

double PjgPolarStereoMath::computeCentralScale(double standard_lat)

{
  return (1.0 + sin(standard_lat * Pjg::Deg2Rad)) / 2.0;
}


