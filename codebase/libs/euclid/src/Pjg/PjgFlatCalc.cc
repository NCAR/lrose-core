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
 * PjgFlatCalc.cc: Class for calculating transformations using a flat
 *                 projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgFlatCalc.hh>
using namespace std;


/**********************************************************************
 * Constructor
 */

PjgFlatCalc::PjgFlatCalc(const double origin_lat, const double origin_lon,
			 const double rotation,
			 const int nx, const int ny, const int nz,
			 const double dx, const double dy, const double dz,
			 const double minx, const double miny,
			 const double minz) :
  PjgCalc(PjgTypes::PROJ_FLAT,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz),
  _rotation(rotation),
  _rotationRad(rotation * RAD_PER_DEG)
{
  setOrigin(origin_lat, origin_lon);
}


/**********************************************************************
 * Destructor
 */

PjgFlatCalc::~PjgFlatCalc()
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

void PjgFlatCalc::latlon2xy(const double lat, const double lon,
			    double  &x, double &y) const
{
  double r, theta_rad;
  double grid_theta;

  _latlon2RTheta(_originColat,
		 _cosOriginColat,
		 _sinOriginColat,
		 _projOriginLon,
		 lat, lon, r, theta_rad);

  grid_theta = theta_rad - _rotationRad;

  x = r * sin(grid_theta);
  y = r * cos(grid_theta);
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgFlatCalc::xy2latlon(const double x, const double y,
			    double &lat, double &lon,
			    const double z /* = -9999.0*/ ) const
{
  double r, theta_rad;
  
  r = sqrt(x * x + y * y);

  if (x == 0.0 && y == 0.0)
    theta_rad = _rotationRad;
  else
    theta_rad = atan2(x, y) + _rotationRad; // rel to TN
  
  _latlonPlusRTheta(_cosOriginColat,
		    _sinOriginColat,
		    _projOriginLonRad,
		    r, theta_rad, lat, lon);
}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgFlatCalc::km2x(const double km) const
{
  return km;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgFlatCalc::x2km(const double x) const
{
  return x;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgFlatCalc::km2xGrid(const double x_km) const
{
  return _km2grid(x_km, _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgFlatCalc::km2yGrid(const double y_km) const
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

double PjgFlatCalc::xGrid2km(const double x_grid,
			     const int y_index) const
{
  return _grid2km(x_grid, _dx);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgFlatCalc::yGrid2km(const double y_grid) const
{
  return _grid2km(y_grid, _dy);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgFlatCalc::print(ostream &stream) const
{
  stream << "PjgFlatCalc info --" << endl;
  PjgCalc::print(stream);
  stream << "\tproj origin latitude: " << _projOriginLat << endl;
  stream << "\tproj origin longitude: " << _projOriginLon << endl;
  stream << "\trotation: " << _rotation << endl;
}
