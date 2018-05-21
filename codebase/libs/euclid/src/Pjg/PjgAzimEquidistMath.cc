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
// PjgAzimEquidistMath.cc
//
// Low-level math for Azimuthal Equidistant projection.
// This is also know at RAL as the Radar FLAT projection.
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
#define TINY_DIST 1.e-2
#define TINY_DBL 1.e-10

////////////////////////
// Constructor

PjgAzimEquidistMath::PjgAzimEquidistMath(double origin_lat,
                                         double origin_lon,
                                         double rotation /* = 0.0 */) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_AZIM_EQUIDIST;
  
  _origin_lat = origin_lat;
  _origin_lon = origin_lon; 
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

  _rotation = rotation;

  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  _rotation_rad = rotation * Pjg::Deg2Rad;
  _origin_colat_rad = (90.0 - _origin_lat) * Pjg::Deg2Rad;
  
  EG_sincos(_origin_lat_rad, &_sin_origin_lat, &_cos_origin_lat);
  EG_sincos(_origin_colat_rad, &_sin_origin_colat, &_cos_origin_colat);

}

///////////////
// print object

void PjgAzimEquidistMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg): " << _origin_lon << endl;
  out << "  origin_lat (deg): " << _origin_lat << endl;
  out << "  rotation (deg)  : " << _rotation << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgAzimEquidistMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived: " << endl;
  out << "    rotation (deg): " << _rotation << endl;
  out << "    origin_lat_rad: " << _origin_lat_rad << endl;
  out << "    origin_lon_rad: " << _origin_lon_rad << endl;
  out << "    origin_colat_rad: " << _origin_colat_rad << endl;
  out << "    rotation_rad: " << _rotation_rad << endl;
  out << "    sin_origin_lat: " << _sin_origin_lat << endl;
  out << "    cos_origin_lat: " << _cos_origin_lat << endl;
  out << "    sin_origin_colat: " << _sin_origin_colat << endl;
  out << "    cos_origin_colat: " << _cos_origin_colat << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgAzimEquidistMath::latlon2xy(double lat, double lon,
                                    double &x, double &y,
                                    double z /* = -9999 */) const
  
{

  if (lat == _origin_lat && lon == _origin_lon) {
    x = _false_easting;
    y = _false_northing;
    return;
  }

  double xx, yy;

  if (fabs(_rotation) > 0.0001) {

    // if we have a grid rotation, use the old code

    double r, theta_rad;
    _latlon_2_r_theta(lat, lon, r, theta_rad);
    
    double grid_theta = theta_rad - _rotation_rad;
    double sin_theta, cos_theta;
    EG_sincos(grid_theta, &sin_theta, &cos_theta);
    
    xx = r * sin_theta;
    yy = r * cos_theta;

  } else {

    // Reference: Map Projections used by the USGS

    double sinLat, cosLat;
    EG_sincos(lat * Pjg::Deg2Rad, &sinLat, &cosLat);
    
    double deltaLonRad = (lon * Pjg::Deg2Rad) - _origin_lon_rad;
    double sinDeltaLon, cosDeltaLon;
    EG_sincos(deltaLonRad, &sinDeltaLon, &cosDeltaLon);
    
    double cosCC = (_sin_origin_lat * sinLat +
                    _cos_origin_lat * cosLat * cosDeltaLon);

    double cc = acos(cosCC);
    double sinCC = sin(cc);

    if (sinCC == 0.0) {
      x = _false_easting;
      y = _false_northing;
      return;
    }
    
    double kk = cc / sinCC;
    double rk = Pjg::EradKm * kk;
    
    xx = rk * cosLat * sinDeltaLon;
    yy = rk * (_cos_origin_lat * sinLat -
               _sin_origin_lat * cosLat * cosDeltaLon);

  }

  x = xx + _false_easting;
  y = yy + _false_northing;
  
}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgAzimEquidistMath::xy2latlon(double x, double y,
                                    double &lat, double &lon,
                                    double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;

  if (fabs(_rotation) > 0.0001) {

    // if we have a grid rotation, use the old code

    double r = sqrt(x * x + y * y);
    
    double theta_rad = 0.0;
    if (x == 0.0 && y == 0.0) {
      theta_rad = _rotation_rad;
    } else {
      theta_rad = atan2(x, y) + _rotation_rad; // rel to TN
    }
    
    _latlon_plus_r_theta(r, theta_rad, lat, lon);

  } else {

    // Reference: Map Projections used by the USGS

    double dist = sqrt(x * x + y * y);
    if (dist == 0.0) {
      lat = _origin_lat_rad * Pjg::Rad2Deg;
      lon = _origin_lon_rad * Pjg::Rad2Deg;
      return;
    }
    
    double cc = dist / Pjg::EradKm;
    double sinCC, cosCC;
    EG_sincos(cc, &sinCC, &cosCC);
    
    double latRad = asin((cosCC * _sin_origin_lat) +
                         (y * sinCC * _cos_origin_lat / dist));
    
    double lonRad = 0.0;
    if (_origin_lat_rad == M_PI_2) {
      lonRad = _origin_lon_rad + atan2(x, -y);
    } else if (_origin_lat_rad == -M_PI_2) {
      lonRad = _origin_lon_rad + atan2(x, y);
    } else {
      lonRad = _origin_lon_rad +
        atan2(x * sinCC,
              dist * _cos_origin_lat * cosCC - y * _sin_origin_lat * sinCC);
    }
    
    lat = latRad * Pjg::Rad2Deg;
    lon = lonRad * Pjg::Rad2Deg;
    lon = conditionRange180(lon);
    conditionLon2Origin(lon);
    
  }
    
}
     
/////////////////////////////////////////////////////////////////////////
// latlon_plus_r_theta()
//
//  Starting from a given lat, lon, draw an arc (part of a great circle)
//  of length r which makes an angle of theta from true North.
//  Theta is positive if east of North, negative (or > PI) if west of North,
//  0 = North
//
//  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
//	    arclength r (km), angle theta (degrees)
//  Output: lat2, lon2, the ending point (degrees)

void PjgAzimEquidistMath::_latlon_plus_r_theta(double r, double theta_rad,
                                               double &lat2, double &lon2) const

{
  
  double darc, colat2;
  double denom, delta_lon;
  double cos_colat2, sin_colat2;
  double sin_darc, cos_darc;
  double sin_theta, cos_theta;
  double xx;

  darc = r / Pjg::EradKm;
  EG_sincos(darc, &sin_darc, &cos_darc);
  EG_sincos(theta_rad, &sin_theta, &cos_theta);

  xx = _cos_origin_colat * cos_darc + _sin_origin_colat * sin_darc * cos_theta;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  EG_sincos(colat2, &sin_colat2, &cos_colat2);
  lat2 = 90.0 - colat2 * Pjg::Rad2Deg;
  
  denom = _sin_origin_colat * sin_colat2;
  
  if ( fabs(denom) <= TINY_DBL) {
    delta_lon = 0.0;
  } else {
    xx = (cos(darc) - _cos_origin_colat * cos_colat2) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    delta_lon = acos(xx);
  }

  /*
   * reverse sign if theta is west
   */

  if (sin_theta < 0.0)
    delta_lon *= -1.0;
  
  lon2 = (_origin_lon_rad + delta_lon) * Pjg::Rad2Deg;

  if (lon2 < -180.0)
    lon2 += 360.0;
  if (lon2 > 180.0)
    lon2 -= 360.0;

  return;
  
}

//////////////////////////////////////////////////////////////////////
// latlon_2_r_theta
//
//  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
//  Output: r = the arc length from 1 to 2, in km 
//	    theta =  angle with True North: positive if east of North,
//          negative if west of North, 0 = North

void PjgAzimEquidistMath::_latlon_2_r_theta(double lat, double lon,
                                            double &r, double &theta_rad) const
  
{

  double darc, colat2, delon, denom, therad;
  double cos_colat2, sin_colat2;
  double sin_darc, cos_darc;
  double xx;

  colat2 = (90.0 - lat) * Pjg::Deg2Rad;

  EG_sincos(colat2, &sin_colat2, &cos_colat2);

  delon = (lon - _origin_lon) * Pjg::Deg2Rad;
  
  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }
  
  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }
  
  xx = _cos_origin_colat * cos_colat2 +
    _sin_origin_colat * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  EG_sincos(darc, &sin_darc, &cos_darc);
  
  r = darc* Pjg::EradKm;
  
  denom = _sin_origin_colat * sin_darc;

  if ((fabs(_origin_colat_rad) <= TINY_ANGLE) || (fabs(denom) <= TINY_DBL)) {
    therad = 0.0;
  } else {
    xx = (cos_colat2 - _cos_origin_colat * cos_darc) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }
  
  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;
  
  theta_rad = therad;

  return;

}

