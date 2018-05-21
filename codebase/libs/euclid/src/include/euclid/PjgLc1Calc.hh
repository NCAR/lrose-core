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

/************************************************************************
 * PjgLc1Calc.hh: Class for calculating transformations using a Lambert
 *                Conformal projection with a single latitude.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PjgLc1Calc_hh
#define PjgLc1Calc_hh

#include <string>

#include <toolsa/toolsa_macros.h>
#include <euclid/PjgCalc.hh>
#include <euclid/PjgTypes.hh>

using namespace std;

class PjgLc1Calc : public PjgCalc
{

public:

  /**********************************************************************
   * Constructors
   */
  
  PjgLc1Calc(const double origin_lat, const double origin_lon,
	     const double lat1,
	     const int nx = 1, const int ny = 1, const int nz = 1,
	     const double dx = 1.0, const double dy = 1.0,
	     const double dz = 1.0,
	     const double minx = 0.0, const double miny = 0.0,
	     const double minz = 0.0);
  
  /**********************************************************************
   * Destructor
   */

  virtual ~PjgLc1Calc();
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * getProjType() - Retrieve the current projection type.
   */

  int getProjType(void) const
  {
    return PjgTypes::PROJ_LC1;
  }
  
  
  /**********************************************************************
   * getOriginLat() - Retrieve the current value of the latitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  inline double getOriginLat(void) const
  {
    return _projOriginLat;
  }
  
  
  /**********************************************************************
   * getOriginLon() - Retrieve the current value of the longitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  inline double getOriginLon(void) const
  {
    return _projOriginLon;
  }
  
  
  /**********************************************************************
   * setOrigin() - Sets the projection origin for the projection, if the
   *               projection uses an origin.  Does nothing for projections
   *               that don't use an origin.
   */

  inline void setOrigin(const double origin_lat,
			const double origin_lon)
  {
    static const string method_name = "PjgLc1Calc::setOrigin()";
    
    double tweeked_origin_lat = origin_lat;
  
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
  
    _projOriginLat = tweeked_origin_lat;
    
    _projOriginLon = origin_lon;
    _originLonRad = origin_lon * RAD_PER_DEG;
  
  }
  
  
  /**********************************************************************
   * getLat1() - Retrieve the current value of the projection lat1.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  inline double getLat1(void) const
  {
    return _lat1;
  }
  
  
  /**********************************************************************
   * getLat2() - Retrieve the current value of the projection lat1.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  inline double getLat2(void) const
  {
    return _lat1;
  }
  
  
  /////////////////////
  // Virtual methods //
  /////////////////////

  /**********************************************************************
   * latlon2xy() - Convert the given lat/lon location to the grid location
   *               in grid units.
   */

  virtual void latlon2xy(const double lat, const double lon,
			 double  &x, double &y) const;

  /**********************************************************************
   * xy2latlon() - Convert the given grid location specified in grid units
   *               to the appropriate lat/lon location.
   */
  
  virtual void xy2latlon(const double x, const double y,
			 double &lat, double &lon,
			 const double z = -9999.0) const;

  /**********************************************************************
   * km2x() - Converts the given distance in kilometers to the same
   *          distance in the units appropriate to the projection.
   */

  virtual double km2x(const double km) const;
  
  /**********************************************************************
   * x2km() - Converts the given distance to kilometers.  The distance
   *          is assumed to be in the units appropriate to the projection.
   */

  virtual double x2km(const double x) const;
  
  /**********************************************************************
   * km2xGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the X axis.
   */

  virtual double km2xGrid(const double x_km) const;
  
  /**********************************************************************
   * km2yGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the Y axis.
   */

  virtual double km2yGrid(const double y_km) const;
  
  /**********************************************************************
   * xGrid2km() - Converts the given distance in number of grid spaces
   *              along the X axis to kilometers.  If y_index is non-negative,
   *              the conversion is done at that point in the grid;
   *              otherwise, the conversion is done at the center of the
   *              grid.
   */

  virtual double xGrid2km(const double x_grid,
			  const int y_index = -1) const;
  
  /**********************************************************************
   * yGrid2km() - Converts the given distance in number of grid spaces
   *              along the Y axis to kilometers.
   */

  virtual double yGrid2km(const double y_grid) const;


  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * print() - Print the projection parameters to the given stream
   */

  virtual void print(ostream &stream) const;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  double _projOriginLat;
  double _projOriginLon;
  
  double _lat1;
  
  double _originLonRad;
  
  double _rho;
  double _tan0;
  double _sin0;
  

  ///////////////////////////////
  // Virtual protected methods //
  ///////////////////////////////

private:

};

#endif
