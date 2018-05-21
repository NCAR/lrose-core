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
// PjgMercatorMath.cc
//
// Low-level math for Mercator projection
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

PjgMercatorMath::PjgMercatorMath(double origin_lat,
                                 double origin_lon) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_MERCATOR;

  _origin_lat = origin_lat;
  _origin_lon = origin_lon; 
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  
  _origin_colat_rad = (90.0 - _origin_lat) * Pjg::Deg2Rad;
  EG_sincos(_origin_colat_rad, &_sin_origin_colat, &_cos_origin_colat);

}

///////////////
// print object

void PjgMercatorMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg): " << _origin_lon << endl;
  out << "  origin_lat (deg): " << _origin_lat << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgMercatorMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    origin_lat_rad  : " << _origin_lat_rad << endl;
  out << "    origin_lon_rad  : " << _origin_lon_rad << endl;
  out << "    origin_colat_rad: " << _origin_colat_rad << endl;
  out << "    sin_origin_colat: " << _sin_origin_colat << endl;
  out << "    cos_origin_colat: " << _cos_origin_colat << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgMercatorMath::latlon2xy(double lat, double lon,
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
  double dlon = lon_rad - _origin_lon_rad;

  double xx = Pjg::EradKm * dlon;
  double yy = Pjg::EradKm * atanh(sin(lat_rad - _origin_lat_rad));

  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgMercatorMath::xy2latlon(double x, double y,
                                double &lat, double &lon,
                                double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;

  double d =  pow(M_E,(-y /Pjg::EradKm)); 
  double lat2 = M_PI_2 - (2 * atan(d));

  lat = (lat2 + _origin_lat_rad) * Pjg::Rad2Deg;
  lon = ((x / Pjg::EradKm) + _origin_lon_rad) * Pjg::Rad2Deg;

}
     
