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
////////////////////////////////////////////////////////////////////////////////
//
// Projection calculations from Mike Dixon's libs/mdv/mdv_proj.c
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <euclid/Projection.hh>
#include <euclid/ProjFlat.hh>
#include <euclid/Pjg.hh>
using namespace std;

//
// static constants
//
const double ProjFlat::TINY_ANGLE = 1.e-4;
const double ProjFlat::TINY_FLOAT = 1.e-10;


ProjFlat::ProjFlat( Projection *parent )
         :ProjType( parent )
{
   clear();
}

void
ProjFlat::clear()
{
   latOriginRad = 0.0;
   lonOriginRad = 0.0;
   rotationRad  = 0.0;
   colatOrigin  = 0.0;
   sinColat     = 0.0;
   cosColat     = 0.0;
}

void
ProjFlat::updateOrigin()
{
   double latOrigin = projection->getLatOrigin();
   double lonOrigin = projection->getLonOrigin();
   double rotation  = projection->getRotation();

   //
   // Initialize the projection coefficients
   //
   latOriginRad = latOrigin * RAD_PER_DEG;
   lonOriginRad = lonOrigin * RAD_PER_DEG;
   rotationRad  = rotation  * RAD_PER_DEG;

   colatOrigin = (90.0 - latOrigin) * RAD_PER_DEG;
 
   sinColat = sin(colatOrigin);
   cosColat = cos(colatOrigin);
}

void
ProjFlat::latlon2xy( double lat, double lon, double *xKm, double *yKm )
{
  double r, theta_rad;
  double grid_theta;

  latlon2Rtheta( colatOrigin, cosColat, sinColat, projection->getLonOrigin(),
                 lat, lon, &r, &theta_rad);

  grid_theta = theta_rad - rotationRad;

  *xKm = r * sin(grid_theta);
  *yKm = r * cos(grid_theta);
}

void
ProjFlat::xy2latlon( double xKm, double yKm, double *lat, double *lon )
{
  double r, theta_rad;

  r = sqrt(xKm * xKm + yKm * yKm);

  if (xKm == 0.0 && yKm == 0.0) {
    theta_rad = rotationRad;
  } else {
    theta_rad = atan2(xKm, yKm) + rotationRad; /* rel to TN */
  }
 
  latlonPlusRtheta( cosColat, sinColat, lonOriginRad,
                    r, theta_rad, lat, lon );
}

void
ProjFlat::latlon2Rtheta( double colat1,
                         double cos_colat1,
                         double sin_colat1,
                         double lon1,
                         double lat2, double lon2,
                         double *r, double *theta_rad )
{
/*********************************************************************
 * latlon_2_r_theta
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km
 *          theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

  double darc, colat2, delon, denom, therad;
  double cos_colat2, sin_colat2;
  double xx;

  colat2 = (90.0 - lat2) * DEG_TO_RAD;

  cos_colat2 = cos(colat2);
  sin_colat2 = sin(colat2);

  delon = (lon2 - lon1) * DEG_TO_RAD;

  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }

  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }

  xx = cos_colat1 * cos_colat2 + sin_colat1 * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);

  *r = darc* Pjg::EradKm;

  denom = sin_colat1 * sin(darc);

  if ((fabs(colat1) <= TINY_ANGLE) || (fabs(denom) <= TINY_FLOAT)) {
    therad = 0.0;
  } else {
    xx = (cos_colat2 - cos_colat1 * cos(darc)) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }

  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;

  *theta_rad = therad;
}

void
ProjFlat::latlonPlusRtheta( double cos_colat1,
                            double sin_colat1,
                            double lon1_rad,
                            double r, double theta_rad,
                            double *lat2, double *lon2)
{
/*******************************************************************
 * latlon_plus_r_theta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *          arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

  double darc, colat2;
  double denom, delta_lon;
  double cos_theta;
  double cos_colat2, sin_colat2;
  double xx;

  darc = r / Pjg::EradKm;
  cos_theta = cos(theta_rad);

  xx = cos_colat1 * cos(darc) + sin_colat1 * sin(darc) * cos_theta;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  cos_colat2 = cos(colat2);
  sin_colat2 = sin(colat2);
  *lat2 = 90.0 - colat2 * RAD_TO_DEG;

  denom = sin_colat1 * sin_colat2;

  if ( fabs(denom) <= TINY_FLOAT) {
    delta_lon = 0.0;
  } else {
    xx = (cos(darc) - cos_colat1 * cos_colat2) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    delta_lon = acos(xx);
  }

  /*
   * reverse sign if theta is west
   */

  if (sin(theta_rad) < 0.0)
    delta_lon *= -1.0;

  *lon2 = (lon1_rad + delta_lon) * RAD_TO_DEG;

  if (*lon2 < -180.0)
    *lon2 += 360.0;
  if (*lon2 > 180.0)
    *lon2 -= 360.0;
}
