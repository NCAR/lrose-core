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
// PjgObliqueStereoMath.cc
//
// Low-level math for Stereographic projection in the
// oblique aspect
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

PjgObliqueStereoMath::PjgObliqueStereoMath(double tangent_lat,
                                           double tangent_lon,
                                           double central_scale /* = 1.0*/) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_OBLIQUE_STEREO;
  
  _origin_lat = tangent_lat;
  _origin_lon = tangent_lon; 

  _tangent_lat = tangent_lat;
  _tangent_lon = tangent_lon;
  _central_scale = central_scale;

  if( _central_scale == 0.0) {
    _central_scale = 1.0;
  }
  
  _tangent_lat_rad = tangent_lat * Pjg::Deg2Rad;
  _tangent_lon_rad = tangent_lon * Pjg::Deg2Rad;
  EG_sincos(_tangent_lat_rad, &_sin_tangent_lat, &_cos_tangent_lat);
  
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

}

///////////////
// print object

void PjgObliqueStereoMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg) : " << _origin_lon << endl;
  out << "  origin_lat (deg) : " << _origin_lat << endl;
  out << "  tangent_lat (deg): " << _tangent_lat << endl;
  out << "  tangent_lon (deg): " << _tangent_lon << endl;
  out << "  central_scale    : " << _central_scale << endl;

  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgObliqueStereoMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    tangent_lat_rad: " << _tangent_lat_rad << endl;
  out << "    tangent_lon_rad: " << _tangent_lon_rad << endl;
  out << "    sin_tangent_lat: " << _sin_tangent_lat << endl;
  out << "    cos_tangent_lat: " << _cos_tangent_lat << endl;
  out << "    false_easting  : " << _false_easting << endl;
  out << "    false_northing : " << _false_northing << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgObliqueStereoMath::latlon2xy(double lat, double lon,
                                     double &x, double &y,
                                     double z /* = -9999 */) const
  
{

  double lat_rad = lat * Pjg::Deg2Rad;
  double lon_rad = lon * Pjg::Deg2Rad;

  double sin_delta_lon, cos_delta_lon;
  EG_sincos(lon_rad - _tangent_lon_rad, &sin_delta_lon, &cos_delta_lon);

  double sin_lat1, cos_lat1;
  EG_sincos(lat_rad, &sin_lat1, &cos_lat1);
  
  double k = 2.0 / (1.0 + _sin_tangent_lat * sin_lat1 + 
		    _cos_tangent_lat * cos_lat1 * cos_delta_lon);
  
  double xx = Pjg::EradKm * k * cos_lat1 * sin_delta_lon;
  xx *= _central_scale;
  x = xx + _false_easting;
  
  double yy =
    Pjg::EradKm * k * ( _cos_tangent_lat * sin_lat1 - _sin_tangent_lat * 
                   cos_lat1 * cos_delta_lon);
  yy *= _central_scale;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgObliqueStereoMath::xy2latlon(double x, double y,
                                     double &lat, double &lon,
                                     double z /* = -9999 */) const
  
{
  
  /* first translate into coord system with origin at tangent point */

  x -= _false_easting;
  y -= _false_northing;
  
  double rho = hypot(x, y);
  double c = 2.0 * atan2( rho, 2.0 * Pjg::EradKm * _central_scale);	
  double sinc, cosc;
  EG_sincos(c, &sinc, &cosc);

  double phi;
  phi = asin(cosc * _sin_tangent_lat + y * sinc * 
             _cos_tangent_lat / rho);
  
  lat = phi * Pjg::Rad2Deg;
  
  double lamda = _tangent_lon_rad + 
    atan2(x * sinc,
          rho *_cos_tangent_lat *cosc - y * sinc *_sin_tangent_lat);

  lon = conditionRange180(lamda * Pjg::Rad2Deg);
  conditionLon2Origin(lon);

}
     
