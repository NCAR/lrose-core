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
// PjgAlbersMath.cc
//
// Low-level math for Albers Equal Area Conic projection
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

PjgAlbersMath::PjgAlbersMath(double origin_lat,
                             double origin_lon,
                             double lat1,
                             double lat2) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_ALBERS;
  
  _origin_lat = origin_lat;
  _origin_lon = origin_lon;
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  
  EG_sincos(_origin_lat_rad, &_sin_origin_lat, &_cos_origin_lat);

  _lat1 = lat1;
  _lat2 = lat2;
  
  double sinLat1, cosLat1;
  EG_sincos(lat1 * Pjg::Deg2Rad, &sinLat1, &cosLat1);

  double sinLat2, cosLat2;
  EG_sincos(lat2 * Pjg::Deg2Rad, &sinLat2, &cosLat2);
  
  _n = (sinLat1 + sinLat2) / 2.0;
  _c = cosLat1 * cosLat1 + 2.0 * _n * sinLat1;
  _rho0 = (Pjg::EradKm * sqrt(_c - 2.0 * _n * _sin_origin_lat) / _n);
  
}

///////////////
// print object

void PjgAlbersMath::print(ostream &out) const

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

void PjgAlbersMath::printDetails(ostream &out) const

{

  print(out);

  out << "  Derived: " << endl;
  out << "    origin_lat_rad: " << _origin_lat_rad << endl;
  out << "    origin_lon_rad: " << _origin_lon_rad << endl;
  out << "    sin_origin_lat: " << _sin_origin_lat << endl;
  out << "    cos_origin_lat: " << _cos_origin_lat << endl;
  out << "    n             : " << _n << endl;
  out << "    c             : " << _c << endl;
  out << "    rho0          : " << _rho0 << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgAlbersMath::latlon2xy(double lat, double lon,
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
  
  double sinLat = sin(lat_rad);

  double deltaLonRad = lon_rad - _origin_lon_rad;
  if (deltaLonRad < -M_PI) {
    deltaLonRad += 2.0 * M_PI;
  } else if (deltaLonRad > M_PI) {
    deltaLonRad -= 2.0 * M_PI;
  }
  double thetaRad = deltaLonRad * _n;
  double sinTheta, cosTheta;
  EG_sincos(thetaRad, &sinTheta, &cosTheta);

  double rho = Pjg::EradKm * sqrt(_c - 2.0 * _n * sinLat) / _n;

  double xx = rho * sinTheta;
  double yy = _rho0 - rho * cosTheta;
  
  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgAlbersMath::xy2latlon(double x, double y,
                              double &lat, double &lon,
                              double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;

  double dy = (_rho0 - y);
  double rho = sqrt((x * x) + (dy * dy));

  double mult = 1.0;
  if (_n < 0) {
    mult = -1.0;
  }
  double theta = atan2(x * mult, dy * mult);

  double aa = (rho * _n / Pjg::EradKm);
  double aa2 = aa * aa;

  double latRad = asin((_c - aa2) / (2.0 * _n));
  double lonRad = _origin_lon_rad + theta / _n;
  
  lat = latRad * Pjg::Rad2Deg;
  lon = lonRad * Pjg::Rad2Deg;
  lon = conditionRange180(lon);
  conditionLon2Origin(lon);

}
     
