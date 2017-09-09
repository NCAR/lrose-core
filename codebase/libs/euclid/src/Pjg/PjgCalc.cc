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
 * PjgCalc.cc: Base class for classes that calculate transformations
 *             for a particular projection.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgCalc.hh>
#include <euclid/PjgFlatCalc.hh>
#include <euclid/PjgLatlonCalc.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgMercatorCalc.hh>
#include <euclid/PjgPolarRadarCalc.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <euclid/PjgObliqueStereoCalc.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

// Static definitions

const double PjgCalc::TINY_ANGLE = 1.e-4;
const double PjgCalc::TINY_DIST  = 1.e-2;
const double PjgCalc::TINY_FLOAT = 1.e-10;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

PjgCalc::PjgCalc(const PjgTypes::proj_type_t proj_type,
		 const int nx, const int ny, const int nz,
		 const double dx, const double dy, const double dz,
		 const double minx, const double miny, const double minz) :
  _projType(proj_type),
  _isConstantNx(true),
  _nx(nx),
  _ny(ny),
  _nz(nz),
  _dx(dx),
  _dy(dy),
  _dz(dz),
  _minx(minx),
  _miny(miny),
  _minz(minz)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

PjgCalc::~PjgCalc()
{
  // Do nothing
}


/////////////////////
// Factory methods //
/////////////////////

/**********************************************************************
 * copyCalc() - Create a new PjgCalc object that is a copy of the
 *              given object.
 *
 * Returns a pointer to the new object on success, 0 on failure.
 */

PjgCalc *PjgCalc::copyCalc(const PjgCalc *orig)
{
  // Create a new calculator based on the type of the old calculator

  const PjgFlatCalc *flat_calc = dynamic_cast< const PjgFlatCalc* >(orig);
  if (flat_calc != 0)
    return new PjgFlatCalc(*flat_calc);
    
  const PjgLatlonCalc *latlon_calc =
    dynamic_cast< const PjgLatlonCalc* >(orig);
  if (latlon_calc != 0)
    return new PjgLatlonCalc(*latlon_calc);
  
  const PjgLc1Calc *lc1_calc = dynamic_cast< const PjgLc1Calc* >(orig);
  if (lc1_calc != 0)
    return new PjgLc1Calc(*lc1_calc);
  
  const PjgLc2Calc *lc2_calc = dynamic_cast< const PjgLc2Calc* >(orig);
  if (lc2_calc != 0)
    return new PjgLc2Calc(*lc2_calc);
  
  const PjgMercatorCalc *mercator_calc =
    dynamic_cast< const PjgMercatorCalc* >(orig);
  if (mercator_calc != 0)
    return new PjgMercatorCalc(*mercator_calc);

  const PjgPolarRadarCalc *polar_calc =
    dynamic_cast< const PjgPolarRadarCalc* >(orig);
  if (polar_calc != 0)
    return new PjgPolarRadarCalc(*polar_calc);

  const PjgPolarStereoCalc *polar_stereo_calc =
    dynamic_cast< const PjgPolarStereoCalc* >(orig);
  if (polar_stereo_calc != 0)
    return new PjgPolarStereoCalc(*polar_stereo_calc);

  const PjgObliqueStereoCalc *oblique_stereo_calc =
    dynamic_cast< const PjgObliqueStereoCalc* >(orig);
  if (oblique_stereo_calc != 0)
    return new PjgObliqueStereoCalc(*oblique_stereo_calc);

  cerr << "Warning: PjgCalc::copyCalc() does not support projection type "
    << orig->getProjType() << endl;

  return 0;
}


////////////////////////////////////
// Generic Transformation Methods //
////////////////////////////////////

/**********************************************************************
 * latlon2RTheta() - Calculate the distance and angle between two
 *                   lat/lon points.
 *
 * Input:  lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 * Output: r = the arc length from 1 to 2, in km
 *         theta = angle with True North: positive if east of North,
 *                 negative if west of North, 0 = North
 */

void PjgCalc::latlon2RTheta(const double lat1, const double lon1,
			    const double lat2, const double lon2,
			    double &r, double &theta)
{

  double darc, colat1, colat2, delon, denom, therad;
  double xx;

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  colat2 = (90.0 - lat2) * DEG_TO_RAD;
  delon = (lon2 - lon1) * DEG_TO_RAD;

  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }
  
  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }

  xx = cos(colat1)*cos(colat2) + sin(colat1)*sin(colat2)*cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  
  r = darc* Pjg::EradKm;
  
  denom = sin(colat1) * sin (darc);

  if ((fabs(colat1) <= TINY_ANGLE) || (fabs(denom) <= TINY_FLOAT)) {
    therad = 0.0;
  } else {
    xx = (cos(colat2) - cos(colat1)*cos(darc)) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }
  
  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;
  
  theta = therad * RAD_TO_DEG;

}

/**********************************************************************
 * latlonPlusRTheta() - Starting from a given lat/lon, draw an arc
 *                      (part of a great circle) of length r which makes
 *                      an angle of theta from true North.  Theta is
 *                      positive if east of North, negative (or > PI) if
 *                      west of North, 0 = North
 *
 * Input:  Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *         arclength r (km), angle theta (degrees)
 * Output: Ending point lat2, lon2 in degrees
 */

void PjgCalc::latlonPlusRTheta(const double lat1, const double lon1,
			       const double r, const double theta,
			       double &lat2, double &lon2)
{

  double darc, colat1, colat2, denom, delta_lon, cost, therad;
  double xx;
  
  darc = r / Pjg::EradKm;
  therad = theta * DEG_TO_RAD;
  cost = cos(therad);

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  
  xx = cos( colat1)*cos( darc) + sin(colat1)*sin(darc)*cost;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  lat2 = 90.0 - colat2 * RAD_TO_DEG;
  
  denom = sin (colat1) * sin(colat2);
  if ( fabs(denom) <= TINY_FLOAT) {
    delta_lon = 0.0;
  } else {
    xx = (cos(darc) - cos(colat1)*cos(colat2))/denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    delta_lon = acos(xx);
  }

  // reverse sign if theta is west

  if (sin(therad) < 0.0) {
    delta_lon *= -1.0;
  }

  lon2 = lon1 + (delta_lon * RAD_TO_DEG);

  if (lon2 < -180.0)
    lon2 += 360.0;
  if (lon2 > 180.0)
    lon2 -= 360.0;
}


/**********************************************************************
 * latlon2xyIndex() - Computes the the data x, y indices for the given
 *                    lat/lon location.
 *
 * Returns 0 on success, -1 on failure (data outside grid)
 */

int PjgCalc::latlon2xyIndex(const double lat, const double lon,
			    int &x_index, int &y_index) const
{
  int iret = 0;
  double x, y;

  latlon2xy(lat, lon, x, y);
  
  x_index = (int)((x - _minx) / _dx + 0.5);
  y_index = (int)((y - _miny) / _dy + 0.5);

  if (x_index < 0 || x_index >= _nx ||
      y_index < 0 || y_index >= _ny)
    iret = -1;

  return iret;
}


/**********************************************************************
 * latlon2arrayIndex() - Computes the index into the data array.
 *
 * Returns 0 on success, -1 on failure (data outside grid).
 */

int PjgCalc::latlon2arrayIndex(const double lat, const double lon,
			       int &array_index) const
{
  int x_index, y_index;

  if (latlon2xyIndex(lat, lon, x_index, y_index))
  {
    array_index = 0;
    return -1;
  }

  array_index = (_nx * y_index) + x_index;
  return 0;
}


/**********************************************************************
 * xyIndex2latlon() - Computes the lat & lon given ix and iy rel to grid.
 */

void PjgCalc::xyIndex2latlon(const int ix, const int iy,
			 double &lat, double &lon) const
{
  double xx = _minx + ix * _dx;
  double yy = _miny + iy * _dy;
  xy2latlon(xx, yy, lat, lon);
}


/**********************************************************************
 * xyIndex2arrayIndex() - Computes the index into the data array.
 *
 * Returns the calculated array index on success, -1 on failure
 * (data outside grid).
 */

int PjgCalc::xyIndex2arrayIndex(const int ix, const int iy, const int iz) const
{
  if (ix < 0 || ix >= _nx ||
      iy < 0 || iy >= _ny ||
      iz < 0 || iz >= _nz)
    return -1;
  
  return ix + (iy * _nx) + (iz * _nx * _ny);
}


/**********************************************************************
 * print() - Print the projection parameters to the given stream
 */

void PjgCalc::print(ostream &stream) const
{
  stream << "\tproj type: " << _projType << endl;
  stream << endl;
  stream << "\tnx: " << _nx << endl;
  stream << "\tny: " << _ny << endl;
  stream << "\tnz: " << _nz << endl;
  stream << "\tdx: " << _dx << endl;
  stream << "\tdy: " << _dy << endl;
  stream << "\tdz: " << _dz << endl;
  stream << "\tmin x: " << _minx << endl;
  stream << "\tmin y: " << _miny << endl;
  stream << "\tmin z: " << _minz << endl;
}


///////////////////////
// protected methods //
///////////////////////

/**********************************************************************
 * _latlonPlusRTheta() - Starting from a given lat, lon, draw an arc
 *                          (part of a great circle) of length r which
 *                          makes an angle of theta from true North.
 *                          Theta is positive if east of North, negative
 *                          (or > PI) if west of North, 0 = North
 *
 * Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *         arclength r (km), angle theta (degrees)
 * Output: lat2, lon2, the ending point (degrees)
 */

void PjgCalc::_latlonPlusRTheta(const double cos_colat1,
				const double sin_colat1,
				const double lon1_rad,
				const double r,
				const double theta_rad,
				double &lat2, double &lon2) const
{
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
  lat2 = 90.0 - colat2 * RAD_TO_DEG;
  
  denom = sin_colat1 * sin_colat2;
  
  if ( fabs(denom) <= TINY_FLOAT)
  {
    delta_lon = 0.0;
  }
  else
  {
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
  
  lon2 = (lon1_rad + delta_lon) * RAD_TO_DEG;

  if (lon2 < -180.0)
    lon2 += 360.0;
  if (lon2 > 180.0)
    lon2 -= 360.0;

  return;
}


/**********************************************************************
 * _latlon2RTheta() - 
 *
 * Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 * Output: r = the arc length from 1 to 2, in km 
 *         theta =  angle with True North: positive if east of North,
 *         negative if west of North, 0 = North
 */

void PjgCalc::_latlon2RTheta(const double colat1,
			     const double cos_colat1,
			     const double sin_colat1,
			     const double lon1,
			     const double lat2,
			     const double lon2,
			     double &r, double &theta_rad) const
{
  double darc, colat2, delon, denom, therad;
  double cos_colat2, sin_colat2;
  double xx;

  colat2 = (90.0 - lat2) * DEG_TO_RAD;

  cos_colat2 = cos(colat2);
  sin_colat2 = sin(colat2);

  delon = (lon2 - lon1) * DEG_TO_RAD;
  
  if (delon < -M_PI)
    delon += 2.0 * M_PI;
  
  if (delon > M_PI)
    delon -= 2.0 * M_PI;
  
  xx = cos_colat1 * cos_colat2 + sin_colat1 * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  
  r = darc* Pjg::EradKm;
  
  denom = sin_colat1 * sin(darc);

  if ((fabs(colat1) <= TINY_ANGLE) || (fabs(denom) <= TINY_FLOAT))
  {
    therad = 0.0;
  }
  else
  {
    xx = (cos_colat2 - cos_colat1 * cos(darc)) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }
  
  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;
  
  theta_rad = therad;
}

/**********************************************************************
 * _km2grid() - Convert the given distance in kilometers to the 
 *		number of grid spaces.
 *
 * Input : km_dist, grid_delta
 * Output: num. grid spaces
 */

double PjgCalc::_km2grid(double km_dist,
			 double grid_delta) const
{
  return km_dist / grid_delta;
}

/**********************************************************************
 * _grid2km() - Convert the given distance in number of grid spaces 
 *		to kilometers.
 *
 * Input : grid_dist, grid_delta
 * Output: distance
 */

double PjgCalc::_grid2km(double grid_dist,
			 double grid_delta) const
{
    return grid_dist * grid_delta;
}
