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
 * PjgPolarStereoCalc.cc: Class for calculating transformations using 
 *                        a polar stereographic projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Gary Cunning
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgPolarStereoCalc::PjgPolarStereoCalc(const double tangent_lon,
				       const PjgTypes::pole_type_t pt /* = PjgTypes::POLE_NORTH */,
				       const double central_scale /* = 1.0 */,
				       const int nx /* = 1 */,
				       const int ny /* = 1 */,
				       const int nz /* = 1 */,
				       const double dx /* = 1.0 */,
				       const double dy /* = 1.0 */,
				       const double dz /* = 1.0 */,
				       const double minx /* = 0.0 */,
				       const double miny /* = 0.0 */,
				       const double minz /* = 0.0 */) :
  PjgCalc(PjgTypes::PROJ_POLAR_STEREO,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  if (pt == PjgTypes::POLE_NORTH)
    _math = new PjgPolarStereoMath(tangent_lon, true, central_scale);
  else
    _math = new PjgPolarStereoMath(tangent_lon, false, central_scale);
  
  _tangentLon = tangent_lon;
  _pole = pt;
  _centralScale = central_scale;
}


/**********************************************************************
 * Destructor
 */

PjgPolarStereoCalc::~PjgPolarStereoCalc()
{
  delete _math;
}


////////////////////////////////////
// Generic Transformation Methods //
////////////////////////////////////

/**********************************************************************
 * latlon2xy() - Convert the given lat/lon location to the grid location
 *               in grid units.
 */

void PjgPolarStereoCalc::latlon2xy(const double lat, const double lon,
				   double  &x, double &y) const
{
  _math->latlon2xy(lat, lon, x, y);
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgPolarStereoCalc::xy2latlon(const double x, const double y,
					 double &lat, double &lon,
					 const double z /* = -9999.0*/ ) const
{
  _math->xy2latlon(x, y, lat, lon, z);
}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgPolarStereoCalc::km2x(const double km) const
{
  return km;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgPolarStereoCalc::x2km(const double x) const
{
  return x;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgPolarStereoCalc::km2xGrid(const double x_km) const
{
  return _km2grid(x_km, _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgPolarStereoCalc::km2yGrid(const double y_km) const
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

double PjgPolarStereoCalc::xGrid2km(const double x_grid,
				    const int y_index) const
{
 return _grid2km(x_grid, _dx);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgPolarStereoCalc::yGrid2km(const double y_grid) const
{
  return _grid2km(y_grid, _dy);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgPolarStereoCalc::print(ostream &stream) const
{
  stream << "PjgPolarStereoCalc info --" << endl;
  PjgCalc::print(stream);
}
