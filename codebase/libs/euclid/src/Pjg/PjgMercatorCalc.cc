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
 * PjgMercatorCalc.cc: Class for calculating transformations
 * using a Mercator projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Carl Drews
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgMercatorCalc.hh>
using namespace std;


/**********************************************************************
 * Constructor
 */

PjgMercatorCalc::PjgMercatorCalc(const double origin_lat, const double origin_lon,
			 const int nx, const int ny, const int nz,
			 const double dx, const double dy, const double dz,
			 const double minx, const double miny,
			 const double minz) :
  PjgCalc(PjgTypes::PROJ_MERCATOR,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  setOrigin(origin_lat, origin_lon);
}


/**********************************************************************
 * Destructor
 */

PjgMercatorCalc::~PjgMercatorCalc()
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

void PjgMercatorCalc::latlon2xy(const double lat, const double lon,
			    double  &x, double &y) const
{
#if	0
  double r, theta_rad;

  _latlon2RTheta(_originColat,
		 _cosOriginColat,
		 _sinOriginColat,
		 _projOriginLon,
		 lat, lon, r, theta_rad);

  x = r * sin(theta_rad);
  y = r * cos(theta_rad);
#else
  x = Pjg::EradKm * (lon - getOriginLon()) * DEG_TO_RAD;
  y = Pjg::EradKm * log(tan((PI + 2 * (lat - getOriginLat()) * DEG_TO_RAD) / 4));
#endif
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgMercatorCalc::xy2latlon(const double x, const double y,
			    double &lat, double &lon,
			    const double z /* = -9999.0*/ ) const
{
#if	0
  double r, theta_rad;
  
  r = sqrt(x * x + y * y);

  if (x == 0.0 && y == 0.0)
    theta_rad = 0.0;
  else
    theta_rad = atan2(x, y); // rel to TN
  
  _latlonPlusRTheta(_cosOriginColat,
		    _sinOriginColat,
		    _projOriginLonRad,
		    r, theta_rad, lat, lon);
#else
  lon = (x / Pjg::EradKm) * RAD_TO_DEG + getOriginLon();
  lat = (PI/2 - 2 * atan(pow(M_E, -y / Pjg::EradKm))) * RAD_TO_DEG + getOriginLat();
#endif
}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgMercatorCalc::km2x(const double km) const
{
  return km;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgMercatorCalc::x2km(const double x) const
{
  return x;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgMercatorCalc::km2xGrid(const double x_km) const
{
  return _km2grid(x_km, _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgMercatorCalc::km2yGrid(const double y_km) const
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

double PjgMercatorCalc::xGrid2km(const double x_grid,
			     const int y_index) const
{
  return _grid2km(x_grid, _dx); 
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgMercatorCalc::yGrid2km(const double y_grid) const
{
  return _grid2km(y_grid, _dy);  
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgMercatorCalc::print(ostream &stream) const
{
  stream << "PjgMercatorCalc info --" << endl;
  PjgCalc::print(stream);
  stream << "\tproj origin latitude: " << _projOriginLat << endl;
  stream << "\tproj origin longitude: " << _projOriginLon << endl;
}
