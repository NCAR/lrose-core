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
// PjgVertPerspMath.cc
//
// Low-level math for Vertical Perspective projection
// This is as a satellite sees the earth
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

////////////////////////
// Constructor

PjgVertPerspMath::PjgVertPerspMath(double origin_lat,
                                   double origin_lon,
                                   double persp_radius) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_VERT_PERSP;
  
  _origin_lat = origin_lat;
  _origin_lon = origin_lon; 
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  
  _persp_radius = persp_radius;
  _p = _persp_radius / Pjg::EradKm;
  
  EG_sincos(_origin_lat_rad, &_sin_origin_lat, &_cos_origin_lat);

}

///////////////
// print object

void PjgVertPerspMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon  (deg): " << _origin_lon << endl;
  out << "  origin_lat  (deg): " << _origin_lat << endl;
  out << "  persp_radius (km): " << _persp_radius << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgVertPerspMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    origin_lat_rad: " << _origin_lat_rad << endl;
  out << "    origin_lon_rad: " << _origin_lon_rad << endl;
  out << "    sin_origin_lat: " << _sin_origin_lat << endl;
  out << "    cos_origin_lat: " << _cos_origin_lat << endl;
  out << "    p             : " << _p << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgVertPerspMath::latlon2xy(double lat, double lon,
                                 double &x, double &y,
                                 double z /* = -9999 */) const
  
{

  if (lat == _origin_lat && lon == _origin_lon) {
    x = _false_easting;
    y = _false_northing;
    return;
  }
  
  double lat_rad = lat * Pjg::Deg2Rad;
  double lon_rad = lon * Pjg::Deg2Rad;
  double sinLat, cosLat;
  EG_sincos(lat_rad, &sinLat, &cosLat);
  
  double deltaLonRad = lon_rad - _origin_lon_rad;
  double sinDeltaLon, cosDeltaLon;
  EG_sincos(deltaLonRad, &sinDeltaLon, &cosDeltaLon);

  double cosC = (_sin_origin_lat * sinLat +
                 _cos_origin_lat * cosLat * cosDeltaLon);
  double denom = _p - cosC;
  if (denom == 0) {
    x = _false_easting;
    y = _false_northing;
    return;
  }
  
  double p_minus_1 = _p - 1.0;
  double kk = p_minus_1 / denom;
  double rk = Pjg::EradKm * kk;

  double xx = rk * cosLat * sinDeltaLon;
  double yy =
    rk * (_cos_origin_lat * sinLat - _sin_origin_lat * cosLat * cosDeltaLon);

  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgVertPerspMath::xy2latlon(double x, double y,
                                 double &lat, double &lon,
                                 double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;

  double rho = sqrt((x * x) + (y * y));
  double p_minus_1 = _p - 1.0;
  double p_plus_1 = _p + 1.0;

  double term1 =
    _p - sqrt(1.0 - (rho * rho * p_plus_1) / (Pjg::EradKm * Pjg::EradKm * p_minus_1));
  double term2 = (Pjg::EradKm * p_minus_1 / rho);
  double term3 = term2 + 1.0 / term2;

  double cc = asin(term1 / term3);
  double sinCC, cosCC;
  EG_sincos(cc, &sinCC, &cosCC);

  double latRad =
    asin(cosCC * _sin_origin_lat + (y * sinCC * _cos_origin_lat) / rho);
  double lonRad = _origin_lon_rad +
    atan2(x * sinCC,
          rho * _cos_origin_lat * cosCC - y * _sin_origin_lat * sinCC);

  lat = latRad * Pjg::Rad2Deg;
  lon = lonRad * Pjg::Rad2Deg;
  lon = conditionRange180(lon);
  conditionLon2Origin(lon);

}
     
