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
// PjgLambertConfMath.cc
//
// Low-level math for Lambert Conformal Conic projection
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
#define VALUE_NOT_SET -9999.0

////////////////////////
// Constructor

PjgLambertConfMath::PjgLambertConfMath(double origin_lat,
                                       double origin_lon,
                                       double lat1,
                                       double lat2) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_LAMBERT_CONF;

  double  t1, t2, t0n, t1n;

  // check illegal values
  
  if (fabs(origin_lat - 90.0) < TINY_ANGLE ||
      fabs(origin_lat + 90.0) < TINY_ANGLE) {
    cerr << "WARNING - PjgLambertConfMath" << endl;
    cerr << "  origin lat is at a pole: " << origin_lat << endl;
    if (fabs(origin_lat - 90.0) < TINY_ANGLE) {
      origin_lat -= TINY_ANGLE;
    } else {
      origin_lat += TINY_ANGLE;
    }
  }
  
  if (fabs(lat1 - 90.0) < TINY_ANGLE ||
      fabs(lat1 + 90.0) < TINY_ANGLE) {
    cerr << "WARNING - PjgLambertConfMath" << endl;
    cerr << "  lat1 is at a pole: " << lat1 << endl;
    if (fabs(lat1 - 90.0) < TINY_ANGLE) {
      lat1 -= TINY_ANGLE;
    } else {
      lat1 += TINY_ANGLE;
    }
  }
  
  if (fabs(lat2 - 90.0) < TINY_ANGLE ||
      fabs(lat2 + 90.0) < TINY_ANGLE) {
    cerr << "WARNING - PjgLambertConfMath" << endl;
    cerr << "  lat2 is at a pole: " << lat2 << endl;
    if (fabs(lat2 - 90.0) < TINY_ANGLE) {
      lat2 -= TINY_ANGLE;
    } else {
      lat2 += TINY_ANGLE;
    }
  }
  
  _2tan_line = true;
  if (fabs(lat2-lat1) < TINY_ANGLE) {
    _2tan_line = false;
  }

  _origin_lat = origin_lat;
  _origin_lon = origin_lon;
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;
  
  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  _origin_colat_rad = (90.0 - _origin_lat) * Pjg::Deg2Rad;

  _lat1 = lat1;
  _lat2 = lat2;
  _lat1_rad = _lat1 * Pjg::Deg2Rad;
  _lat2_rad = _lat2 * Pjg::Deg2Rad;

  if (_2tan_line) {

    t1 = tan( M_PI_4 + _lat1_rad / 2);
    t2 = tan( M_PI_4 + _lat2_rad / 2);
    _n = log( cos(_lat1_rad)/cos(_lat2_rad))
      / log(t2/t1);
    
    t1n = pow(t1, _n);
    _F = cos(_lat1_rad) * t1n / _n;
    
    t0n = pow( tan(M_PI_4 + _origin_lat_rad/2), _n);
    _rho = Pjg::EradKm * _F / t0n;
    _sin0 = VALUE_NOT_SET;
    _tan0 = VALUE_NOT_SET;

  } else {

    _sin0 = sin(_lat1_rad);
    _tan0 = tan( M_PI_4 - _lat1_rad / 2);
    _rho = Pjg::EradKm / tan(_lat1_rad);
    _n = VALUE_NOT_SET;
    _F = VALUE_NOT_SET;
  }
  
}

///////////////
// print object

void PjgLambertConfMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg): " << _origin_lon << endl;
  out << "  origin_lat (deg): " << _origin_lat << endl;
  out << "  lat1 (deg)      : " << _lat1 << endl;
  out << "  lat2 (deg)      : " << _lat2 << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgLambertConfMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    origin_lat_rad: " << _origin_lat_rad << endl;
  out << "    origin_lon_rad: " << _origin_lon_rad << endl;
  out << "    origin_colat_rad: " << _origin_colat_rad << endl;
  out << "    lat1_rad: " << _lat1_rad << endl;
  out << "    lat2_rad: " << _lat2_rad << endl;
  out << "    n: " << _n << endl;
  out << "    F: " << _F << endl;
  out << "    rho: " << _rho << endl;
  out << "    tan0: " << _tan0 << endl;
  out << "    sin0: " << _sin0 << endl;
  out << "    2tan_line: " << _2tan_line << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgLambertConfMath::latlon2xy(double lat, double lon,
                                   double &x, double &y,
                                   double z /* = -9999 */) const
  
{

  if (lat == _origin_lat && lon == _origin_lon) {
    x = _false_easting;
    y = _false_northing;
    return;
  }

  double xx, yy;

  if (_2tan_line) {
    _latlon2xy_2tan(lat, lon, xx, yy);
  }
  else {
    _latlon2xy_1tan(lat, lon, xx, yy);
  }

  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgLambertConfMath::xy2latlon(double x, double y,
                                   double &lat, double &lon,
                                   double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;
  
  if (_2tan_line) {
    _xy2latlon_2tan(x, y, lat, lon);
  }
  else {
    _xy2latlon_1tan(x, y, lat, lon);
  }
  
  conditionLon2Origin(lon);

}
     
///////////////////////////
// lambert latlon2xy_2tan()

void PjgLambertConfMath::_latlon2xy_2tan(double lat, double lon,
                                         double &x, double &y) const
  
{

  double lat_rad = lat * Pjg::Deg2Rad;
  double lon_rad = lon * Pjg::Deg2Rad;

  double theta = _n * (lon_rad - _origin_lon_rad);

  double tn = pow( tan(M_PI_4 + lat_rad / 2.0), _n);
  double r = Pjg::EradKm * _F / tn;

  double sin_theta, cos_theta;
  EG_sincos(theta, &sin_theta, &cos_theta);
  x = r * sin_theta;
  y = _rho - r * cos_theta;

}

///////////////////////////
// lambert latlon2xy_1tan()

void PjgLambertConfMath::_latlon2xy_1tan(double lat, double lon,
                                         double &x, double &y) const
  
{

  double lat_rad = lat * Pjg::Deg2Rad;
  double lon_rad = lon * Pjg::Deg2Rad;

  double tan_phi = tan(M_PI_4 - lat_rad / 2);
  double f1 = pow((tan_phi/_tan0), _sin0);
  double del_lon = lon_rad - _origin_lon_rad;
  double sin_lon, cos_lon;
  EG_sincos(del_lon * _sin0, &sin_lon, &cos_lon);

  x = _rho * f1 * sin_lon;
  y = _rho * (1 - f1 * cos_lon);

}

///////////////////////////
// lambert xy2latlon_2tan()

void PjgLambertConfMath::_xy2latlon_2tan(double x, double y,
                                         double &lat, double &lon) const
  
{

  double r, theta, rn, yd;

  if (_n < 0.0) {
	yd = (-_rho + y);
    theta = atan2(-x, yd);
    r = sqrt(x * x + yd * yd);
    r *= -1.0;
  } else {
    yd = (_rho - y);
    theta = atan2(x, yd);
    r = sqrt(x * x + yd * yd);
  }

  lon = (theta / _n + _origin_lon_rad) * Pjg::Rad2Deg;
  lon = conditionRange180(lon);

  if (fabs(r) < TINY_DBL) {
    lat = ((_n < 0.0) ? -90.0 : 90.0);
  }
  else {
    rn = pow( Pjg::EradKm * _F / r, 1 / _n);
    lat = (2.0 * atan(rn) - M_PI_2) * Pjg::Rad2Deg;
  }

  lat = conditionRange180(lat);

}

///////////////////////////
// lambert xy2latlon_1tan()

void PjgLambertConfMath::_xy2latlon_1tan(double x, double y,
                                         double &lat, double &lon) const
  
{

  double del_lon, sin_lon, f1, r;
  double inv_sin0, to_sin0, loc_x;

  loc_x = x;

  inv_sin0 = 1/_sin0;

  if (fabs(loc_x) < TINY_DBL) {
    loc_x = 0.001;
  }

  del_lon = inv_sin0*atan2(loc_x,(_rho - y));

  lon = _origin_lon_rad + del_lon;
  
  sin_lon = sin(del_lon * _sin0);
  r = _rho * sin_lon;
  to_sin0 = pow((loc_x/r), inv_sin0);
  f1 = 2*atan(_tan0 * to_sin0);

  lon = conditionRange180(lon*Pjg::Rad2Deg);
 
  lat = (M_PI_2 - f1) * Pjg::Rad2Deg;
  lat = conditionRange180(lat);

}

