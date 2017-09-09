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
 * PjgObliqueStereoCalc.cc: Class for calculating transformations 
 *                          using a oblique stereographic projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Gary Cunning
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgObliqueStereoCalc.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgObliqueStereoCalc::PjgObliqueStereoCalc(const double origin_lat,
					   const double origin_lon,
					   const double tangent_lat,
					   const double tangent_lon,
					   const int nx /* = 1 */,
					   const int ny /* = 1 */,
					   const int nz /* = 1 */,
					   const double dx /* = 1.0 */,
					   const double dy /* = 1.0 */,
					   const double dz /* = 1.0 */,
					   const double minx /* = 0.0 */,
					   const double miny /* = 0.0 */,
					   const double minz /* = 0.0 */) :
  PjgCalc(PjgTypes::PROJ_STEREOGRAPHIC,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

PjgObliqueStereoCalc::~PjgObliqueStereoCalc()
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

void PjgObliqueStereoCalc::latlon2xy(const double lat, const double lon,
			      double  &x, double &y) const
{
  y = lat;
  x = lon;
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgObliqueStereoCalc::xy2latlon(const double x, const double y,
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

double PjgObliqueStereoCalc::km2x(const double km) const
{
  return km / KM_PER_DEG_AT_EQ;
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgObliqueStereoCalc::x2km(const double x) const
{
  return x * KM_PER_DEG_AT_EQ;
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgObliqueStereoCalc::km2xGrid(const double x_km) const
{
  double mid_lat = _miny + _dy * _ny / 2.0;
  double latitude_factor = cos(mid_lat * DEG_TO_RAD);
  
  return _km2grid((x_km * DEG_PER_KM_AT_EQ / latitude_factor), _dx);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgObliqueStereoCalc::km2yGrid(const double y_km) const
{
  return _km2grid((y_km * DEG_PER_KM_AT_EQ), _dy);
}


/**********************************************************************
 * xGrid2km() - Converts the given distance in number of grid spaces
 *              along the X axis to kilometers.  If y_index is non-negative,
 *              the conversion is done at that point in the grid;
 *              otherwise, the conversion is done at the center of the
 *              grid.
 */

double PjgObliqueStereoCalc::xGrid2km(const double x_grid,
			       const int y_index) const
{
  double base_lat;
  
  if (y_index < 0)
    base_lat = _miny + _dy * _ny / 2.0;
  else
    base_lat = _miny + _dy * y_index;
  
  double latitude_factor = cos(base_lat * DEG_TO_RAD);
  
  return _grid2km((x_grid * KM_PER_DEG_AT_EQ * latitude_factor), _dx);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgObliqueStereoCalc::yGrid2km(const double y_grid) const
{
  return _grid2km((y_grid * KM_PER_DEG_AT_EQ), _dy);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgObliqueStereoCalc::print(ostream &stream) const
{
  stream << "PjgObliqueStereoCalc info --" << endl;
  PjgCalc::print(stream);
}
