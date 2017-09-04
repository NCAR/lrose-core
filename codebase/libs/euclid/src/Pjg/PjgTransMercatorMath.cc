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
// PjgTransMercatorMath.cc
//
// Low-level math for Tranverse Mercator projection
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

PjgTransMercatorMath::PjgTransMercatorMath(double origin_lat,
                                           double origin_lon,
                                           double central_scale /* = 1.0*/) :
        PjgMath()
  
{

  _proj_type = PjgTypes::PROJ_TRANS_MERCATOR;

  _origin_lat = origin_lat;
  _origin_lon = origin_lon; 

  _origin_lat_rad = origin_lat * Pjg::Deg2Rad;
  _origin_lon_rad = origin_lon * Pjg::Deg2Rad;
  
  _central_scale = central_scale;
  
  _offset_lat = _origin_lat;
  _offset_lon = _origin_lon;

}

///////////////
// print object

void PjgTransMercatorMath::print(ostream &out) const

{
  
  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  out << "  origin_lon (deg): " << _origin_lon << endl;
  out << "  origin_lat (deg): " << _origin_lat << endl;
  out << "  central_scale   : " << _central_scale << endl;
  
  printOffsetOrigin(out);
  
}

///////////////////////////////
// print object for debugging

void PjgTransMercatorMath::printDetails(ostream &out) const

{

  print(out);
  out << "  Derived:" << endl;
  out << "    origin_lat_rad: " << _origin_lat_rad << endl;
  out << "    origin_lon_rad: " << _origin_lon_rad << endl;

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection

void PjgTransMercatorMath::latlon2xy(double lat, double lon,
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

  double deltaLonRad = lon_rad - _origin_lon_rad;
  double sinDeltaLon, cosDeltaLon;
  EG_sincos(deltaLonRad, &sinDeltaLon, &cosDeltaLon);
  
  double scaledRadius = Pjg::EradKm * _central_scale;
  double B = cos(lat_rad) * sinDeltaLon; // (8-5)

  double xx = scaledRadius * atanh(B); // (8-2)

  // (8-3)
  double yy = scaledRadius * (atan(tan(lat_rad) / cosDeltaLon) - _origin_lat_rad);

  x = xx + _false_easting;
  y = yy + _false_northing;

}

/// convert x/y to lat/lon
/// lat/lon in deg
/// x, y in km
/// z ignored for this projection
  
void PjgTransMercatorMath::xy2latlon(double x, double y,
                                     double &lat, double &lon,
                                     double z /* = -9999 */) const
  
{

  x -= _false_easting;
  y -= _false_northing;
  
  // correct for offsets, compute angular position for x,y

  double scaledRadius = Pjg::EradKm * _central_scale;
  double xxRad = x / scaledRadius;
  double yyRad = y / scaledRadius;
  
  double D = yyRad + _origin_lat_rad; // (8-8)
  double sinD, cosD;
  EG_sincos(D, &sinD, &cosD);

  double latRad = asin(sinD / cosh(xxRad));
  double lonRad = _origin_lon_rad + atan2(sinh(xxRad), cosD);  // (8-7)

  lat = latRad * Pjg::Rad2Deg;
  lon = lonRad * Pjg::Rad2Deg;

}
     
