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
// PjgPolarRadarMath.cc
//
// Low-level math for Polar Radar projection.
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

PjgPolarRadarMath::PjgPolarRadarMath(double origin_lat,
                                     double origin_lon,
                                     double rotation /* = 0.0 */) :
        PjgAzimEquidistMath(origin_lat, origin_lon, rotation)
  
{

  _proj_type = PjgTypes::PROJ_POLAR_RADAR;

}

/////////////////////////////////////////////
/// Set offset origin by specifying lat/lon.
/// Not applicable to this projection.

void PjgPolarRadarMath::setOffsetOrigin(double offset_lat,
                                        double offset_lon)
  
{

}

/////////////////////////////////////////////////////////////////////
/// Set offset origin by specifying false_northing and false_easting.
/// Not applicable to this projection.

void PjgPolarRadarMath::setOffsetCoords(double false_northing,
                                        double false_easting)

{

}
  
///////////////////////
// LatLon conversions

// convert lat/lon to x/y
// x: range in km
// y: azimuth in degrees
// z: elevation angle in degrees
  
void PjgPolarRadarMath::latlon2xy(double lat, double lon,
                                  double &x, double &y,
                                  double z /* = -9999 */) const
  
{

  if (lat == _origin_lat && lon == _origin_lon) {
    x = 0;
    y = 0;
    return;
  }

  double r, theta_rad;
  _latlon_2_r_theta(lat, lon, r, theta_rad);
  
  if (fabs(z - -9999.0) > 0.0001) {
    x = r / cos(z * Pjg::Deg2Rad);
  } else {
    x = r;
  }

  y = theta_rad * Pjg::Rad2Deg;
  if (y < 0) {
    y += 360.0;
  }

}

// convert x/y to lat/lon
// x: range in km
// y: azimuth in degrees
// z: elevation angle in degrees
  
void PjgPolarRadarMath::xy2latlon(double x, double y,
                                  double &lat, double &lon,
                                  double z /* = -9999 */) const
  
{

  double r;
  if (fabs(z - -9999.0) > 0.0001) {
    r = x * cos(z * Pjg::Deg2Rad);
  } else {
    r = x;
  }
  
  double theta_rad = y * Pjg::Deg2Rad;
  _latlon_plus_r_theta(r, theta_rad, lat, lon);
    
}
     
