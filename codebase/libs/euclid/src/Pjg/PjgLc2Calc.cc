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
/*********************************************************************
 * PjgLc2Calc.cc: Class for calculating transformations using a Lambert
 *                Conformal projection with 2 tangent latitudes.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string>

#include <euclid/Pjg.hh>
#include <euclid/PjgLc2Calc.hh>
#include <toolsa/toolsa_macros.h>
#include <cassert>
using namespace std;

static double _range180( double a)
{
  while (a < -180.)
    a += 360.;
  while (a > 180.)
    a -= 360.;
  return a;
}

//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgLc2Calc::PjgLc2Calc(const double origin_lat, const double origin_lon,
		       const double lat1, const double lat2,
		       const int nx, const int ny, const int nz,
		       const double dx, const double dy, const double dz,
		       const double minx, const double miny,
		       const double minz) :
  PjgCalc(PjgTypes::PROJ_LC2,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz),
  _projOriginLat(origin_lat),
  _projOriginLon(origin_lon)
{
  const string method_name = "PjgLc2Calc::Constructor";
  
  double  t1, t2, t0n, t1n;
  
  double tweeked_origin_lat = origin_lat;

  _lat1 = lat1;
  _lat2 = lat2;
  
  // check illegal values
  
  if (fabs(tweeked_origin_lat - 90.0) < TINY_ANGLE ||
      fabs(tweeked_origin_lat + 90.0) < TINY_ANGLE)
  {
    cerr << "WARNING - " << method_name << endl;
    cerr << "  origin lat is at a pole: " << tweeked_origin_lat << endl;
    if (fabs(tweeked_origin_lat - 90.0) < TINY_ANGLE)
      tweeked_origin_lat -= TINY_ANGLE;
    else
      tweeked_origin_lat += TINY_ANGLE;
  }
  
  if (fabs(_lat1 - 90.0) < TINY_ANGLE ||
      fabs(_lat1 + 90.0) < TINY_ANGLE)
  {
    cerr << "WARNING - " << method_name << endl;
    cerr << "  lat1 is at a pole: " << _lat1 << endl;
    if (fabs(_lat1 - 90.0) < TINY_ANGLE)
      _lat1 -= TINY_ANGLE;
    else
      _lat1 += TINY_ANGLE;
  }
  
  if (fabs(_lat2 - 90.0) < TINY_ANGLE ||
      fabs(_lat2 + 90.0) < TINY_ANGLE)
  {
    cerr << "WARNING - " << method_name << endl;
    cerr << "  lat2 is at a pole: " << _lat2 << endl;
    if (fabs(_lat2 - 90.0) < TINY_ANGLE)
      _lat2 -= TINY_ANGLE;
    else
      _lat2 += TINY_ANGLE;
  }
  
  assert (fabs(_lat2 - _lat1) > TINY_ANGLE);

  double origin_lat_rad = tweeked_origin_lat * RAD_PER_DEG;
  _originLonRad = origin_lon * RAD_PER_DEG;
  
  double lc2_lat1_rad = _lat1 * RAD_PER_DEG;
  double lc2_lat2_rad = _lat2 * RAD_PER_DEG;

  t1 = tan( M_PI_4 + lc2_lat1_rad / 2);
  t2 = tan( M_PI_4 + lc2_lat2_rad / 2);
  _n = log( cos(lc2_lat1_rad)/cos(lc2_lat2_rad))
    / log(t2/t1);
    
  t1n = pow(t1, _n);
  _F = cos(lc2_lat1_rad) * t1n / _n;
    
  t0n = pow( tan(M_PI_4 + origin_lat_rad/2), _n);
  _rho = Pjg::EradKm * _F / t0n;
}


/**********************************************************************
 * Destructor
 */

PjgLc2Calc::~PjgLc2Calc()
{
  // Do nothing
}


////////////////////////////////////
// Generic Transformation Methods //
////////////////////////////////////

/**********************************************************************
 * latlon2xy() - Convert the given lat/lon location to the grid location
 *               in grid units.
 */

void PjgLc2Calc::latlon2xy(const double lat, const double lon,
			    double  &x, double &y) const
{
  double r, theta, tn;
  double lat_rad, lon_rad;
  
  lat_rad = lat * DEG_TO_RAD;
  lon_rad = lon * DEG_TO_RAD;

  theta = _n * (lon_rad - _originLonRad);

  tn = pow( tan(M_PI_4 + lat_rad / 2), _n);
  r = Pjg::EradKm * _F / tn;

  x = r * sin (theta);
  y = _rho - r * cos(theta);
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgLc2Calc::xy2latlon(const double x, const double y,
			    double &lat, double &lon,
			    const double z /* = -9999.0*/ ) const
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

  lon = (theta / _n + _originLonRad) * RAD_TO_DEG;
  lon = _range180(lon);

  if (fabs(r) < TINY_FLOAT)
  {
    lat = ((_n < 0.0) ? -90.0 : 90.0);
  }
  else
  {
    rn = pow( Pjg::EradKm * _F / r, 1 / _n);
    lat = (2.0 * atan(rn) - M_PI_2) * RAD_TO_DEG;
  }

  lat = _range180(lat);

}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgLc2Calc::km2x(const double km) const
{
  return km;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgLc2Calc::x2km(const double x) const
{
  return x;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgLc2Calc::km2xGrid(const double x_km) const
{
  return _km2grid(x_km, _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgLc2Calc::km2yGrid(const double y_km) const
{
  return _km2grid(y_km, _dy);
}


/**********************************************************************
 * xGrid2km() - Converts the given distance in number of grid spaces
 *              along the X axis to kilometers.  If y_index is non-negative,
 *              the conversion is done at that point in the grid;
 *              otherwise, the conversion is done at the center of the
 *              grid.
 */

double PjgLc2Calc::xGrid2km(const double x_grid,
			    const int y_index) const
{
  return _grid2km(x_grid, _dx);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgLc2Calc::yGrid2km(const double y_grid) const
{
  return _grid2km(y_grid, _dy);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgLc2Calc::print(ostream &stream) const
{
  stream << "PjgLc2Calc info --" << endl;
  PjgCalc::print(stream);
  stream << "\tproj origin latitude: " << _projOriginLat << endl;
  stream << "\tproj origin longitude: " << _projOriginLon << endl;
  stream << "\tlat 1: " << _lat1 << endl;
  stream << "\tlat 2: " << _lat2 << endl;
}
