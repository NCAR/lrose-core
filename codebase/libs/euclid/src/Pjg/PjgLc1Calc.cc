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
 * PjgLc1Calc.cc: Class for calculating transformations using a Lambert
 *                Conformal projection with a single latitude.
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
#include <euclid/PjgLc1Calc.hh>
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

PjgLc1Calc::PjgLc1Calc(const double origin_lat, const double origin_lon,
		       const double lat1,
		       const int nx, const int ny, const int nz,
		       const double dx, const double dy, const double dz,
		       const double minx, const double miny,
		       const double minz) :
  PjgCalc(PjgTypes::PROJ_LC1,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  const string method_name = "PjgLc1Calc::Constructor";
  
  setOrigin(origin_lat, origin_lon);
  
  _lat1 = lat1;
  
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
  
  double lc2_lat1_rad = _lat1 * RAD_PER_DEG;

  _sin0 = sin(lc2_lat1_rad);
  _tan0 = tan( M_PI_4 - lc2_lat1_rad / 2);
  _rho = Pjg::EradKm / tan(lc2_lat1_rad);
}


/**********************************************************************
 * Destructor
 */

PjgLc1Calc::~PjgLc1Calc()
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

void PjgLc1Calc::latlon2xy(const double lat, const double lon,
			    double  &x, double &y) const
{
  double del_lon, f1, tan_phi;
  double sin_lon, cos_lon;
  double lat_rad, lon_rad;

  lat_rad = lat * DEG_TO_RAD;
  lon_rad = lon * DEG_TO_RAD;

  tan_phi = tan(M_PI_4 - lat_rad / 2);
  f1 = pow((tan_phi/_tan0), _sin0);
  del_lon = lon_rad - _originLonRad;
  sin_lon = sin(del_lon * _sin0);
  cos_lon = cos(del_lon * _sin0);

  x = _rho * f1 * sin_lon;

  y = _rho * (1 - f1 * cos_lon);
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgLc1Calc::xy2latlon(const double x, const double y,
			    double &lat, double &lon,
			    const double z /* = -9999.0*/ ) const
{
  double del_lon, sin_lon, f1, r;
  double inv_sin0, to_sin0, loc_x;

  loc_x = x;

  inv_sin0 = 1/_sin0;

  if (fabs(loc_x) < TINY_FLOAT)
    loc_x = 0.001;

  del_lon = inv_sin0*atan2(loc_x,(_rho - y));

  lon = _originLonRad + del_lon;
  
  sin_lon = sin(del_lon * _sin0);
  r = _rho * sin_lon;
  to_sin0 = pow((loc_x/r), inv_sin0);
  f1 = 2*atan(_tan0 * to_sin0);

  lon = _range180((lon*RAD_TO_DEG));
 
  lat = (M_PI_2 - f1) * RAD_TO_DEG;
  lat = _range180(lat);

}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgLc1Calc::km2x(const double km) const
{
  return km;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgLc1Calc::x2km(const double x) const
{
  return x;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgLc1Calc::km2xGrid(const double x_km) const
{
  return _km2grid(x_km, _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgLc1Calc::km2yGrid(const double y_km) const
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

double PjgLc1Calc::xGrid2km(const double x_grid,
			    const int y_index /* = -1 */) const
{
  return _grid2km(x_grid, _dx);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgLc1Calc::yGrid2km(const double y_grid) const
{
  return _grid2km(y_grid, _dy);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgLc1Calc::print(ostream &stream) const
{
  stream << "PjgLc1Calc info --" << endl;
  PjgCalc::print(stream);
  stream << "\tproj origin latitude: " << _projOriginLat << endl;
  stream << "\tproj origin longitude: " << _projOriginLon << endl;
  stream << "\tlat 1: " << _lat1 << endl;
}
