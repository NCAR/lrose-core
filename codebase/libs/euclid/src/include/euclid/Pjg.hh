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
#include <vector>
#include <euclid/PjgTypes.hh>

class PjgCalc;
using namespace std;

class Pjg
{

public:

  /// EARTH RADIUS, DEG/RAD conversions
  
  static double EradKm; /* default is 6378.137 */
  static const double Rad2Deg;
  static const double Deg2Rad;

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
   * setEarthRadiusKm() - overrides the default earth radius in km
   */
  
  static void setEarthRadiusKm(double earthRadiusKm);

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

  virtual int getProjType() const;
  
  /**********************************************************************
   * isConstantNx() - Retrieve the flag indicating whether the grid has a
   *                  constant nx value.
   */

  virtual bool isConstantNx();

  /**********************************************************************
   * getGridDims() - Retrieve the current grid dimensions.  Returns a -1
   *                 for nx if the grid doesn't have a constant nx value.
   */

  virtual void getGridDims(int &nx, int &ny, int &nz) const;

  /**********************************************************************
   * getGridDims() - Retrieve the current grid dimensions with a non-
   *                 constant nx.
   */

  virtual void getGridDims(vector< int > &nx_list, int &ny, int &nz) const;

  /**********************************************************************
   * setGridDims() - Set the current grid dimensions.
   */

  virtual void setGridDims(const int nx, const int ny, const int nz);
  
  /**********************************************************************
   * setGridDims() - Set the current grid dimensions with a non-constant
   *                 nx.
   */

  virtual void setGridDims(const vector< int > nx_list,
			   const int ny, const int nz);
  
  /**********************************************************************
   * getNx() - Retrieve the current value of nx.  Returns a -1 if the
   *           projection doesn't have a constant nx.
   */

  virtual int getNx() const;
  
  /**********************************************************************
   * getNxList() - Retrieve the current list of nx values.
   */

  virtual vector< int > getNxList() const;
  
  /**********************************************************************
   * getNy() - Retrieve the current value of ny.
   */

  virtual int getNy() const;
  
  /**********************************************************************
   * getNz() - Retrieve the current value of nz.
   */

  virtual int getNz() const;
  
  /**********************************************************************
   * getGridDeltas() - Retrieve the current grid deltas.
   */

  virtual void getGridDeltas(double &dx, double &dy, double &dz) const;
  
  /**********************************************************************
   * setGridDeltas() - Set the current grid deltas.
   */

  virtual void setGridDeltas(const double dx, const double dy, const double dz);

  /**********************************************************************
   * getDx() - Retrieve the current value of dx.
   */

  virtual double getDx() const;
  
  /**********************************************************************
   * getDy() - Retrieve the current value of dy.
   */

  virtual double getDy() const;
  
  /**********************************************************************
   * getDz() - Retrieve the current value of dz.
   */

  virtual double getDz() const;

  /**********************************************************************
   * getGridMins() - Retrieve the current grid minimums.
   */

  virtual void getGridMins(double &minx, double &miny, double &minz) const;

  /**********************************************************************
   * setGridMins() - Set the current grid minimums.
   */

  virtual void setGridMins(const double minx, const double miny,
			   const double minz);
  
  /**********************************************************************
   * getMinx() - Retrieve the current value of minx.
   */

  virtual double getMinx() const;
  
  /**********************************************************************
   * getMiny() - Retrieve the current value of miny.
   */

  virtual double getMiny() const;
  
  /**********************************************************************
   * getMinz() - Retrieve the current value of minz.
   */

  virtual double getMinz() const;

  /**********************************************************************
   * getOriginLat() - Retrieve the current value of the latitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual double getOriginLat() const;
  
  /**********************************************************************
   * getOriginLon() - Retrieve the current value of the longitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual double getOriginLon() const;

  /**********************************************************************
   * setOrigin() - Sets the projection origin for the projection, if the
   *               projection uses an origin.  Does nothing for projections
   *               that don't use an origin.
   */

  virtual void setOrigin(const double origin_lat,
                         const double origin_lon);
  
  /**********************************************************************
   * getRotation() - Retrieve the current value of the projection rotation.
   *                 For projections that don't support a rotation value,
   *                 0.0 will be returned.
   */

  virtual double getRotation() const;

  /**********************************************************************
   * getPole() - Retrieve the current value of the pole (north or south).
   *                 For projections that don't support a pole,
   *                 POLE_NORTH will be returned.
   */

  virtual PjgTypes::pole_type_t getPole() const;

  /**********************************************************************
   * getLat1() - Retrieve the current value of the projection lat1.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual double getLat1() const;
  
  /**********************************************************************
   * getLat2() - Retrieve the current value of the projection lat2.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual double getLat2() const;
  
  ///////////////////////////////////////////////
  // Generic coordinate transformation methods //
  ///////////////////////////////////////////////


  /**********************************************************************
   * getLL() - Calculate the lower left lat/lon of the grid
   *
   */

  void getLL(double &LLlat, double &LLlon) const;

  /**********************************************************************
   * getUR() - Calculate the upper right lat/lon of the grid
   *
   */

  void getUR(double &URlat, double &URlon) const;

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
			    double &r, double &theta);
  
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
			       double &lat2, double &lon2);
  
  static void latlonPlusRTheta(const float lat1, const float lon1,
			       const float r, const float theta,
			       float &lat2, float &lon2);
  
  /**********************************************************************
   * latlon2xy() - Convert the given lat/lon location to the grid location
   *               in grid units.
   */

  void latlon2xy(const double lat, const double lon,
		 double  &x, double &y) const;

  /**********************************************************************
   * xy2latlon() - Convert the given grid location specified in grid units
   *               to the appropriate lat/lon location.
   *
   * Z is only used if set for the polar radar projection.
   */
  
  void xy2latlon(const double x, const double y,
		 double &lat, double &lon,
		 const double z = -9999.0) const;

  void xy2latlon(const float x, const float y,
		 float &lat, float &lon,
		 const float z = -9999.0) const;

  /**********************************************************************
   * latlon2xyIndex() - Computes the the data x, y indices for the given
   *                    lat/lon location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  int latlon2xyIndex(const double lat, const double lon,
		     int &x_index, int &y_index) const;

  /**********************************************************************
   * latlon2arrayIndex() - Computes the index into the data array.
   *
   * Returns 0 on success, -1 on failure (data outside grid).
   */

  int latlon2arrayIndex(const double lat, const double lon,
			int &array_index) const;

  /**********************************************************************
   * xyIndex2latlon() - Computes the lat & lon given ix and iy rel to grid.
   */

  void xyIndex2latlon(const int ix, const int iy,
		      double &lat, double &lon) const;
  
  /**********************************************************************
   * km2x() - Converts the given distance in kilometers to the same
   *          distance in the units appropriate to the projection.
   */

  double km2x(const double km) const;
  
  /**********************************************************************
   * x2km() - Converts the given distance to kilometers.  The distance
   *          is assumed to be in the units appropriate to the projection.
   */

  double x2km(const double x) const;
  
  /**********************************************************************
   * km2xGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the X axis.
   */

  double km2xGrid(const double x_km) const;
  
  /**********************************************************************
   * km2yGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the Y axis.
   */

  double km2yGrid(const double y_km) const;
  
  /**********************************************************************
   * xGrid2km() - Converts the given distance in number of grid spaces
   *              along the X axis to kilometers.  If y_index is non-negative,
   *              the conversion is done at that point in the grid;
   *              otherwise, the conversion is done at the center of the
   *              grid.
   */

  double xGrid2km(const double x_grid,
		  const int y_index = -1) const;
  
  /**********************************************************************
   * yGrid2km() - Converts the given distance in number of grid spaces
   *              along the Y axis to kilometers.
   */

  double yGrid2km(const double y_grid) const;
  
  /**********************************************************************
   * xy2xyIndex() - Computes the the data x, y indices for the given
   *                grid units location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  int xy2xyIndex(const double x, const double y,
		 int &x_index, int &y_index) const;

  /**********************************************************************
   * xyIndex2arrayIndex() - Computes the index into the data array.
   *
   * Returns the calculated array index on success, -1 on failure
   * (data outside grid).
   */

  int xyIndex2arrayIndex(const int ix, const int iy, const int iz = 0) const;

  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * print() - Print the projection parameters to the given stream
   */

  void print(ostream &stream) const;

  ///////////////
  // Operators //
  ///////////////

  const Pjg& operator=(const Pjg &rhs);
  bool operator==(const Pjg &other) const;
  bool operator!=(const Pjg &other) const;

protected:

  PjgCalc *_calculator;
  
  
private:

};

#endif
