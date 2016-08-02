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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:19:28 $
 *   $Id: Pjg.hh,v 1.24 2016/03/03 18:19:28 dixon Exp $
 *   $Revision: 1.24 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Pjg.hh: class implementing projective geometry transformations.
 *
 * If you use the default constructor, the projection will be set 
 * to latlon. You must call one of the init() functions if you want
 * alternative behavior.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Pjg_hh
#define Pjg_hh

#include <iostream>

#include <euclid/PjgCalc.hh>
#include <euclid/PjgTypes.hh>

using namespace std;

class Pjg
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**********************************************************************
   * Constructors
   */

  Pjg();
  
  Pjg(const Pjg &rhs);


  /**********************************************************************
   * Destructor
   */

  virtual ~Pjg();
  

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * initFlat() - Initialize flat earth projection.
   */
  
  void initFlat(const double origin_lat, const double origin_lon,
		const double rotation = 0.0,
		const int nx = 1, const int ny = 1, const int nz = 1,
		const double dx = 1.0, const double dy = 1.0,
		const double dz = 1.0,
		const double minx = 0.0, const double miny = 0.0,
		const double minz = 0.0);

  /**********************************************************************
   * initPolarRadar() - Initialize polar radar projection.
   */

  void initPolarRadar(const double origin_lat, const double origin_lon,
		      const int nx = 1, const int ny = 1, const int nz = 1,
		      const double dx = 1.0, const double dy = 1.0,
		      const double dz = 1.0,
		      const double minx = 0.0, const double miny = 0.0,
		      const double minz = 0.0);
  
  /**********************************************************************
   * initLc1() - Initialize lambert conformal projection with one lats.
   */

  void initLc1(const double origin_lat, const double origin_lon,
	       const double lat1,
	       const int nx = 1, const int ny = 1, const int nz = 1,
	       const double dx = 1.0, const double dy = 1.0,
	       const double dz = 1.0,
	       const double minx = 0.0, const double miny = 0.0,
	       const double minz = 0.0);
  
  /**********************************************************************
   * initLc2() - Initialize lambert conformal projection with two lats.
   *             If this method is given two latitudes that are not
   *             tangent, the projection is initialized as an LC1 projection
   *             rather than an LC2 projection.
   */

  void initLc2(const double origin_lat, const double origin_lon,
	       const double lat1, const double lat2,
	       const int nx = 1, const int ny = 1, const int nz = 1,
	       const double dx = 1.0, const double dy = 1.0,
	       const double dz = 1.0,
	       const double minx = 0.0, const double miny = 0.0,
	       const double minz = 0.0);

  /**********************************************************************
   * initLatlon() - Initialize latlon projection.
   */

  void initLatlon(const int nx = 1, const int ny = 1, const int nz = 1,
		  const double dx = 1.0, const double dy = 1.0,
		  const double dz = 1.0,
		  const double minx = 0.0, const double miny = 0.0,
		  const double minz = 0.0);


  /**********************************************************************
   * initPolarStereo() - Initialize polar stereographic projection.
   */

  void initPolarStereo(const double tangent_lon,
		       const PjgTypes::pole_type_t pt = PjgTypes::POLE_NORTH,
		       const double central_scale = 1.0,
		       const int nx = 1, const int ny = 1, const int nz = 1,
		       const double dx = 1.0, const double dy = 1.0,
		       const double dz = 1.0,
		       const double minx = 0.0, const double miny = 0.0,
		       const double minz = 0.0);
  

  /**********************************************************************
   * initObliqueStereo() - Initialize oblique stereographic projection.
   */

  void initObliqueStereo(const double origin_lat, const double origin_lon,
			 const double tangent_lat, const double tangent_lon,
			 const int nx = 1, const int ny = 1, const int nz = 1,
			 const double dx = 1.0, const double dy = 1.0,
			 const double dz = 1.0,
			 const double minx = 0.0, const double miny = 0.0,
			 const double minz = 0.0);

  /**********************************************************************
   * initMercator() - Initialize Mercator projection.
   */
  
  void initMercator(const double origin_lat, const double origin_lon,
		const int nx = 1, const int ny = 1, const int nz = 1,
		const double dx = 1.0, const double dy = 1.0,
		const double dz = 1.0,
		const double minx = 0.0, const double miny = 0.0,
		const double minz = 0.0);


  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * getProjType() - Retrieve the current PjgTypes projection type.
   */

  virtual const int getProjType(void) const
  {
    return _calculator->getProjType();
  }
  
  
  /**********************************************************************
   * isConstantNx() - Retrieve the flag indicating whether the grid has a
   *                  constant nx value.
   */

  virtual bool isConstantNx()
  {
    return _calculator->isConstantNx();
  }
  
  
  /**********************************************************************
   * getGridDims() - Retrieve the current grid dimensions.  Returns a -1
   *                 for nx if the grid doesn't have a constant nx value.
   */

  virtual void getGridDims(int &nx, int &ny, int &nz) const
  {
    _calculator->getGridDims(nx, ny, nz);
  }
  
  
  /**********************************************************************
   * getGridDims() - Retrieve the current grid dimensions with a non-
   *                 constant nx.
   */

  virtual void getGridDims(vector< int > &nx_list, int &ny, int &nz) const
  {
    _calculator->getGridDims(nx_list, ny, nz);
  }
  
  
  /**********************************************************************
   * setGridDims() - Set the current grid dimensions.
   */

  virtual void setGridDims(const int nx, const int ny, const int nz)
  {
    _calculator->setGridDims(nx, ny, nz);
  }
  
  
  /**********************************************************************
   * setGridDims() - Set the current grid dimensions with a non-constant
   *                 nx.
   */

  virtual void setGridDims(const vector< int > nx_list,
			   const int ny, const int nz)
  {
    _calculator->setGridDims(nx_list, ny, nz);
  }
  
  
  /**********************************************************************
   * getNx() - Retrieve the current value of nx.  Returns a -1 if the
   *           projection doesn't have a constant nx.
   */

  virtual inline int getNx(void) const
  {
    return _calculator->getNx();
  }
  
  
  /**********************************************************************
   * getNxList() - Retrieve the current list of nx values.
   */

  virtual inline vector< int > getNxList(void) const
  {
    return _calculator->getNxList();
  }
  
  
  /**********************************************************************
   * getNy() - Retrieve the current value of ny.
   */

  virtual inline int getNy(void) const
  {
    return _calculator->getNy();
  }
  
  
  /**********************************************************************
   * getNz() - Retrieve the current value of nz.
   */

  virtual inline int getNz(void) const
  {
    return _calculator->getNz();
  }
  
  
  /**********************************************************************
   * getGridDeltas() - Retrieve the current grid deltas.
   */

  virtual void getGridDeltas(double &dx, double &dy, double &dz) const
  {
    _calculator->getGridDeltas(dx, dy, dz);
  }
  
  
  /**********************************************************************
   * setGridDeltas() - Set the current grid deltas.
   */

  virtual void setGridDeltas(const double dx, const double dy, const double dz)
  {
    _calculator->setGridDeltas(dx, dy, dz);
  }
  
  
  /**********************************************************************
   * getDx() - Retrieve the current value of dx.
   */

  virtual inline double getDx(void) const
  {
    return _calculator->getDx();
  }
  
  
  /**********************************************************************
   * getDy() - Retrieve the current value of dy.
   */

  virtual inline double getDy(void) const
  {
    return _calculator->getDy();
  }
  
  
  /**********************************************************************
   * getDz() - Retrieve the current value of dz.
   */

  virtual inline double getDz(void) const
  {
    return _calculator->getDz();
  }
  
  
  /**********************************************************************
   * getGridMins() - Retrieve the current grid minimums.
   */

  virtual void getGridMins(double &minx, double &miny, double &minz) const
  {
    _calculator->getGridMins(minx, miny, minz);
  }
  
  
  /**********************************************************************
   * setGridMins() - Set the current grid minimums.
   */

  virtual void setGridMins(const double minx, const double miny,
			   const double minz)
  {
    _calculator->setGridMins(minx, miny, minz);
  }
  
  
  /**********************************************************************
   * getMinx() - Retrieve the current value of minx.
   */

  virtual inline double getMinx(void) const
  {
    return _calculator->getMinx();
  }
  
  
  /**********************************************************************
   * getMiny() - Retrieve the current value of miny.
   */

  virtual inline double getMiny(void) const
  {
    return _calculator->getMiny();
  }
  
  
  /**********************************************************************
   * getMinz() - Retrieve the current value of minz.
   */

  virtual inline double getMinz(void) const
  {
    return _calculator->getMinz();
  }
  
  
  /**********************************************************************
   * getOriginLat() - Retrieve the current value of the latitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual inline double getOriginLat(void) const
  {
    return _calculator->getOriginLat();
  }
  
  
  /**********************************************************************
   * getOriginLon() - Retrieve the current value of the longitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual inline double getOriginLon(void) const
  {
    return _calculator->getOriginLon();
  }
  
  
  /**********************************************************************
   * setOrigin() - Sets the projection origin for the projection, if the
   *               projection uses an origin.  Does nothing for projections
   *               that don't use an origin.
   */

  virtual inline void setOrigin(const double origin_lat,
				const double origin_lon)
  {
    _calculator->setOrigin(origin_lat, origin_lon);
  }
  
  
  /**********************************************************************
   * getRotation() - Retrieve the current value of the projection rotation.
   *                 For projections that don't support a rotation value,
   *                 0.0 will be returned.
   */

  virtual inline double getRotation(void) const
  {
    return _calculator->getRotation();
  }


  /**********************************************************************
   * getPole() - Retrieve the current value of the pole (north or south).
   *                 For projections that don't support a pole,
   *                 POLE_NORTH will be returned.
   */

  virtual inline PjgTypes::pole_type_t getPole(void) const
  {
    return _calculator->getPole();
  }


  /**********************************************************************
   * getLat1() - Retrieve the current value of the projection lat1.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual inline double getLat1(void) const
  {
    return _calculator->getLat1();
  }
  
  
  /**********************************************************************
   * getLat2() - Retrieve the current value of the projection lat2.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual inline double getLat2(void) const
  {
    return _calculator->getLat2();
  }
  
  
  ///////////////////////////////////////////////
  // Generic coordinate transformation methods //
  ///////////////////////////////////////////////


  /**********************************************************************
   * getLL() - Calculate the lower left lat/lon of the grid
   *
   */

  void getLL(double &LLlat, double &LLlon) const
  {
    double LLx, LLy;
    LLx = getMinx() - getDx() / 2.0;
    LLy = getMiny() - getDy() / 2.0;
    xy2latlon(LLx, LLy, LLlat, LLlon);
  }

  /**********************************************************************
   * getUR() - Calculate the upper right lat/lon of the grid
   *
   */

  void getUR(double &URlat, double &URlon) const
  {
    double URx, URy;
    URx = getMinx() + (((double)getNx() - 0.5) * getDx());
    URy = getMiny() + (((double)getNy() - 0.5) * getDy());
    xy2latlon(URx, URy, URlat, URlon);
  }


  /**********************************************************************
   * latlon2RTheta() - Calculate the distance and angle between two
   *                   lat/lon points.
   *
   * Input:  lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
   * Output: r = the arc length from 1 to 2, in km
   *         theta = angle with True North: positive if east of North,
   *                 negative if west of North, 0 = North
   */

  static void latlon2RTheta(const double lat1, const double lon1,
			    const double lat2, const double lon2,
			    double &r, double &theta)
  {
    PjgCalc::latlon2RTheta(lat1, lon1, lat2, lon2, r, theta);
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

  static void latlonPlusRTheta(const double lat1, const double lon1,
			       const double r, const double theta,
			       double &lat2, double &lon2)
  {
    PjgCalc::latlonPlusRTheta(lat1, lon1, r, theta, lat2, lon2);
  }
  
  
  static void latlonPlusRTheta(const float lat1, const float lon1,
			       const float r, const float theta,
			       float &lat2, float &lon2)
  {
    double lat1_double = lat1;
    double lon1_double = lon1;
    double r_double = r;
    double theta_double = theta;
    
    double lat2_double, lon2_double;
    
    latlonPlusRTheta(lat1_double, lon1_double, r_double, theta_double,
		     lat2_double, lon2_double);

    lat2 = lat2_double;
    lon2 = lon2_double;
  }
  
  
  /**********************************************************************
   * latlon2xy() - Convert the given lat/lon location to the grid location
   *               in grid units.
   */

  void latlon2xy(const double lat, const double lon,
		 double  &x, double &y) const
  {
    _calculator->latlon2xy(lat, lon, x, y);
  }
  

  /**********************************************************************
   * xy2latlon() - Convert the given grid location specified in grid units
   *               to the appropriate lat/lon location.
   *
   * Z is only used if set for the polar radar projection.
   */
  
  void xy2latlon(const double x, const double y,
		 double &lat, double &lon,
		 const double z = -9999.0) const
  {
    _calculator->xy2latlon(x, y, lat, lon, z);
  }
  

  void xy2latlon(const float x, const float y,
		 float &lat, float &lon,
		 const float z = -9999.0) const
  {
    double lat_double, lon_double;
    
    _calculator->xy2latlon(x, y, lat_double, lon_double, z);

    lat = (float)lat_double;
    lon = (float)lon_double;
  }
  

  /**********************************************************************
   * latlon2xyIndex() - Computes the the data x, y indices for the given
   *                    lat/lon location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  int latlon2xyIndex(const double lat, const double lon,
		     int &x_index, int &y_index) const
  {
    return _calculator->latlon2xyIndex(lat, lon, x_index, y_index);
  }
  

  /**********************************************************************
   * latlon2arrayIndex() - Computes the index into the data array.
   *
   * Returns 0 on success, -1 on failure (data outside grid).
   */

  int latlon2arrayIndex(const double lat, const double lon,
			int &array_index) const
  {
    return _calculator->latlon2arrayIndex(lat, lon, array_index);
  }
  

  /**********************************************************************
   * xyIndex2latlon() - Computes the lat & lon given ix and iy rel to grid.
   */

  void xyIndex2latlon(const int ix, const int iy,
		      double &lat, double &lon) const
  {
    _calculator->xyIndex2latlon(ix, iy, lat, lon);
  }
  
  
  /**********************************************************************
   * km2x() - Converts the given distance in kilometers to the same
   *          distance in the units appropriate to the projection.
   */

  double km2x(const double km) const
  {
    return _calculator->km2x(km);
  }
  
  
  /**********************************************************************
   * x2km() - Converts the given distance to kilometers.  The distance
   *          is assumed to be in the units appropriate to the projection.
   */

  double x2km(const double x) const
  {
    return _calculator->x2km(x);
  }
  
  
  /**********************************************************************
   * km2xGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the X axis.
   */

  double km2xGrid(const double x_km) const
  {
    return _calculator->km2xGrid(x_km);
  }
  
  
  /**********************************************************************
   * km2yGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the Y axis.
   */

  double km2yGrid(const double y_km) const
  {
    return _calculator->km2yGrid(y_km);
  }
  
  
  /**********************************************************************
   * xGrid2km() - Converts the given distance in number of grid spaces
   *              along the X axis to kilometers.  If y_index is non-negative,
   *              the conversion is done at that point in the grid;
   *              otherwise, the conversion is done at the center of the
   *              grid.
   */

  double xGrid2km(const double x_grid,
		  const int y_index = -1) const
  {
    return _calculator->xGrid2km(x_grid, y_index);
  }
  
  
  /**********************************************************************
   * yGrid2km() - Converts the given distance in number of grid spaces
   *              along the Y axis to kilometers.
   */

  double yGrid2km(const double y_grid) const
  {
    return _calculator->yGrid2km(y_grid);
  }
  
  
  /**********************************************************************
   * xy2xyIndex() - Computes the the data x, y indices for the given
   *                grid units location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  int xy2xyIndex(const double x, const double y,
		 int &x_index, int &y_index) const
  {
    return _calculator->xy2xyIndex(x, y, x_index, y_index);
  }
  

  /**********************************************************************
   * xyIndex2arrayIndex() - Computes the index into the data array.
   *
   * Returns the calculated array index on success, -1 on failure
   * (data outside grid).
   */

  int xyIndex2arrayIndex(const int ix, const int iy, const int iz = 0) const
  {
    return _calculator->xyIndex2arrayIndex(ix, iy, iz);
  }
  

  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * print() - Print the projection parameters to the given stream
   */

  void print(ostream &stream) const
  {
    _calculator->print(stream);
  }
  

  ///////////////
  // Operators //
  ///////////////

  const Pjg& operator=(const Pjg &rhs)
  {
    if (&rhs == this)
      return *this;
    
    delete _calculator;
    _calculator = PjgCalc::copyCalc(rhs._calculator);

    return *this;
  }
  
  bool operator==(const Pjg &other) const
  {
    return *_calculator == *other._calculator;
  }
  
  bool operator!=(const Pjg &other) const
  {
    return !(*_calculator == *other._calculator);
  }
  

protected:

  PjgCalc *_calculator;
  
  
private:

};

#endif
