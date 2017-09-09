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
 * PjgLatlonCalc.cc: Class for calculating transformations using a
 *                   lat/lon projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgLatlonCalc.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgLatlonCalc::PjgLatlonCalc(const int nx, const int ny, const int nz,
			     const double dx, const double dy, const double dz,
			     const double minx, const double miny,
			     const double minz) :
  PjgCalc(PjgTypes::PROJ_LATLON,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

PjgLatlonCalc::~PjgLatlonCalc()
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

void PjgLatlonCalc::latlon2xy(const double lat, const double lon,
			      double  &x, double &y) const
{
  y = lat;
  x = lon;

  // Normalize the longitude

  while (x < _minx)
    x += 360.0;
  while (x >= _minx + 360.0)
    x -= 360.0;
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgLatlonCalc::xy2latlon(const double x, const double y,
			      double &lat, double &lon,
			      const double z /* = -9999.0*/ ) const
{
  lat = y;
  lon = x;
}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgLatlonCalc::km2x(const double km) const
{
  return km / KM_PER_DEG_AT_EQ;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgLatlonCalc::x2km(const double x) const
{
  return x * KM_PER_DEG_AT_EQ;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgLatlonCalc::km2xGrid(const double x_km) const
{
  double mid_lat = _miny + _dy * _ny / 2.0;
  double latitude_factor = cos(mid_lat * DEG_TO_RAD);
  
  return (x_km * DEG_PER_KM_AT_EQ / latitude_factor) / _dx;
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgLatlonCalc::km2yGrid(const double y_km) const
{
  return (y_km * DEG_PER_KM_AT_EQ) / _dy;
}


/**********************************************************************
 * xGrid2km() - Converts the given distance in number of grid spaces
 *              along the X axis to kilometers.  If y_index is non-negative,
 *              the conversion is done at that point in the grid;
 *              otherwise, the conversion is done at the center of the
 *              grid.
 */

double PjgLatlonCalc::xGrid2km(const double x_grid,
			       const int y_index) const
{
  double base_lat;
  
  if (y_index < 0)
    base_lat = _miny + _dy * _ny / 2.0;
  else
    base_lat = _miny + _dy * y_index;
  
  double latitude_factor = cos(base_lat * DEG_TO_RAD);
  
  return (x_grid * KM_PER_DEG_AT_EQ * latitude_factor) * _dx;
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgLatlonCalc::yGrid2km(const double y_grid) const
{
  return (y_grid * KM_PER_DEG_AT_EQ) * _dy;
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgLatlonCalc::print(ostream &stream) const
{
  stream << "PjgLatlonCalc info --" << endl;
  PjgCalc::print(stream);
}
