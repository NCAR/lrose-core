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
 * PjgPolarRadarCalc.cc: Class for calculating transformations using a
 *                       polar radar projection.
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
#include <euclid/PjgPolarRadarCalc.hh>
#include <cassert>
using namespace std;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgPolarRadarCalc::PjgPolarRadarCalc(const double origin_lat,
				     const double origin_lon,
				     const int nx, const int ny, const int nz,
				     const double dx, const double dy,
				     const double dz,
				     const double minx, const double miny,
				     const double minz) :
  PjgCalc(PjgTypes::PROJ_POLAR_RADAR,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz)
{
  setOrigin(origin_lat, origin_lon);
}


/**********************************************************************
 * Destructor
 */

PjgPolarRadarCalc::~PjgPolarRadarCalc()
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

void PjgPolarRadarCalc::latlon2xy(const double lat, const double lon,
				  double  &x, double &y) const
{
  double r, theta_rad;

  _latlon2RTheta(_originColat,
		 _cosOriginColat,
		 _sinOriginColat,
		 _originLon,
		 lat, lon, r, theta_rad);

  x = r;
  y = theta_rad * RAD_TO_DEG;
}


/**********************************************************************
 * latlon2xyIndex() - Computes the the data x, y indices for the given
 *                    lat/lon location.
 *
 * Returns 0 on success, -1 on failure (data outside grid)
 */

int PjgPolarRadarCalc::latlon2xyIndex(const double lat, const double lon,
				      int &x_index, int &y_index) const
{
  int iret = 0;
  double range, theta_deg;

  latlon2xy(lat, lon, range, theta_deg);
  while (theta_deg < _miny)
    theta_deg += 360.0;
  
  x_index = (int)((range - _minx) / _dx + 0.5);
  y_index = (int)((theta_deg - _miny) / _dy + 0.5);
  
  if (x_index < 0 || x_index >= _nx ||
      y_index < 0 || y_index >= _ny)
    iret = -1;

  return iret;
}


/**********************************************************************
 * xy2latlon() - Convert the given grid location specified in grid units
 *               to the appropriate lat/lon location.
 */

void PjgPolarRadarCalc::xy2latlon(const double x, const double y,
			    double &lat, double &lon,
			    const double z /* = -9999.0*/ ) const
{
  double r, theta_rad;
  
  if (z != -9999.0)
    r = x * cos(z * DEG_TO_RAD);
  else
    r = x;

  theta_rad = y * DEG_TO_RAD;

  _latlonPlusRTheta(_cosOriginColat,
		    _sinOriginColat,
		    _originLonRad,
		    r, theta_rad, lat, lon);
}


/**********************************************************************
 * km2x() - Converts the given distance in kilometers to the same
 *          distance in the units appropriate to the projection.
 */

double PjgPolarRadarCalc::km2x(const double km) const
{
  const string method_name = "PjgPolarRadarCalc::km2x()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type" << endl;
  
  assert(0);
}

  
/**********************************************************************
 * x2km() - Converts the given distance to kilometers.  The distance
 *          is assumed to be in the units appropriate to the projection.
 */

double PjgPolarRadarCalc::x2km(const double x) const
{
  const string method_name = "PjgPolarRadarCalc::x2km()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type" << endl;
  
  assert(0);
}


/**********************************************************************
 * km2xGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the X axis.
 */

double PjgPolarRadarCalc::km2xGrid(const double x_km) const
{
  const string method_name = "PjgPolarRadarCalc::km2xGrid()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type" << endl;
  
  assert(0);
}


/**********************************************************************
 * km2yGrid() - Converts the given distance in kilometers to the
 *              appropriate number of grid spaces along the Y axis.
 */

double PjgPolarRadarCalc::km2yGrid(const double y_km) const
{
  const string method_name = "PjgPolarRadarCalc::km2yGrid()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type" << endl;
  
  assert(0);
}


/**********************************************************************
 * xGrid2km() - Converts the given distance in number of grid spaces
 *              along the X axis to kilometers.  If y_index is non-negative,
 *              the conversion is done at that point in the grid;
 *              otherwise, the conversion is done at the center of the
 *              grid.
 */

double PjgPolarRadarCalc::xGrid2km(const double x_grid,
				   const int y_index) const
{
  const string method_name = "PjgPolarRadarCalc::xGrid2km()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type" << endl;
  
  assert(0);
}


/**********************************************************************
 * yGrid2km() - Converts the given distance in number of grid spaces
 *              along the Y axis to kilometers.
 */

double PjgPolarRadarCalc::yGrid2km(const double y_grid) const
{
  const string method_name = "PjgPolarRadarCalc::yGrid2km()";
  
  cerr << "WARNING - " << method_name << endl;
  cerr << "  Unsupported proj type: polar radar" << endl;
  
  assert(0);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgPolarRadarCalc::print(ostream &stream) const
{
  stream << "PjgPolarRadarCalc info --" << endl;
  PjgCalc::print(stream);
  stream << "\torigin latitude: " << _originLat << endl;
  stream << "\torigin longitude: " << _originLon << endl;
}
