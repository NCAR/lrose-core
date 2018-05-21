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
 * PjgCalc.hh: class implementing projective geometry transformations.
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

#ifndef PjgCalc_hh
#define PjgCalc_hh

#include <iostream>
#include <vector>

#include <euclid/PjgMath.hh>
#include <euclid/PjgTypes.hh>


class PjgCalc
{

public:

  static const double TINY_ANGLE;


  /**********************************************************************
   * Constructors
   */

  PjgCalc(const PjgTypes::proj_type_t proj_type = PjgTypes::PROJ_UNKNOWN,
	  const int nx = 1,
	  const int ny = 1,
	  const int nz = 1,
	  const double dx = 1.0,
	  const double dy = 1.0,
	  const double dz = 1.0,
	  const double minx = 0.0,
	  const double miny = 0.0,
	  const double minz = 0.0);
  
  
  /**********************************************************************
   * Destructor
   */

  virtual ~PjgCalc();
  

  /////////////////////
  // Factory methods //
  /////////////////////

  /**********************************************************************
   * copyCalc() - Create a new PjgCalc object that is a copy of the
   *              given object.
   *
   * Returns a pointer to the new object on success, 0 on failure.
   */

  static PjgCalc *copyCalc(const PjgCalc *orig);
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * getProjType() - Retrieve the current PjgTypes projection type.
   */

  virtual int getProjType(void) const = 0;
  
  
  /**********************************************************************
   * isConstantNx() - Retrieve the flag indicating whether the grid has a
   *                  constant nx value.
   */

  virtual bool isConstantNx()
  {
    return _isConstantNx;
  }
  
  
  /**********************************************************************
   * setGridDims() - Set the grid dimensions.
   */

  void setGridDims(const int nx, const int ny, const int nz)
  {
    _nx = nx;
    _ny = ny;
    _nz = nz;

    _isConstantNx = true;
  }
  
  /**********************************************************************
   * setGridDims() - Set the grid dimensions.
   */

  void setGridDims(const vector <int> nx_list, const int ny, const int nz)
  {
    _nxList = nx_list;
    _ny = ny;
    _nz = nz;

    _isConstantNx = true;
  }
  
  /**********************************************************************
   * getGridDims() - Retrieve the grid dimensions.
   */

  void getGridDims(int &nx, int &ny, int &nz) const
  {
    if (_isConstantNx)
      nx = _nx;
    else
      nx = -1;
    
    ny = _ny;
    nz = _nz;
  }
  

  /**********************************************************************
   * getGridDims() - Retrieve the grid dimensions.
   */

  void getGridDims(vector< int > &nx_list, int &ny, int &nz) const
  {
    if (_isConstantNx)
      nx_list.push_back(_nx);
    else
      nx_list = _nxList;
    
    ny = _ny;
    nz = _nz;
  }
  

  /**********************************************************************
   * getNx() - Retrieve the current value of nx.
   */

  inline int getNx(void) const
  {
    int nx = -1;
    
    if (_isConstantNx)
      nx = _nx;
    
    return nx;
  }
  

  /**********************************************************************
   * getNxList() - Retrieve the current list of nx values.
   */

  inline vector< int > getNxList(void) const
  {
    vector< int > nx_list;
    
    if (_isConstantNx)
      nx_list.push_back(_nx);
    else
      nx_list = _nxList;
    
    return nx_list;
  }
  

  /**********************************************************************
   * getNy() - Retrieve the current value of ny.
   */

  inline int getNy(void) const
  {
    return _ny;
  }
  

  /**********************************************************************
   * getNz() - Retrieve the current value of nz.
   */

  inline int getNz(void) const
  {
    return _nz;
  }
  

  /**********************************************************************
   * setGridDeltas() - Set the grid deltas.
   */

  void setGridDeltas(const double dx, const double dy, const double dz)
  {
    _dx = dx;
    _dy = dy;
    _dz = dz;
  }
  
  /**********************************************************************
   * getGridDeltas() - Retrieve the grid deltas.
   */

  void getGridDeltas(double &dx, double &dy, double &dz) const
  {
    dx = _dx;
    dy = _dy;
    dz = _dz;
  }
  

  /**********************************************************************
   * getDx() - Retrieve the current value of dx.
   */

  inline double getDx(void) const
  {
    return _dx;
  }
  

  /**********************************************************************
   * getDy() - Retrieve the current value of dy.
   */

  inline double getDy(void) const
  {
    return _dy;
  }
  

  /**********************************************************************
   * getDz() - Retrieve the current value of dz.
   */

  inline double getDz(void) const
  {
    return _dz;
  }
  

  /**********************************************************************
   * setGridMins() - Set the grid minimums.
   */

  void setGridMins(const double minx, const double miny, const double minz)
  {
    _minx = minx;
    _miny = miny;
    _minz = minz;
  }
  
  /**********************************************************************
   * getGridMins() - Retrieve the grid minimums.
   */

  void getGridMins(double &minx, double &miny, double &minz) const
  {
    minx = _minx;
    miny = _miny;
    minz = _minz;
  }
  

  /**********************************************************************
   * getMinx() - Retrieve the current value of minx.
   */

  inline double getMinx(void) const
  {
    return _minx;
  }
  

  /**********************************************************************
   * getMiny() - Retrieve the current value of miny.
   */

  inline double getMiny(void) const
  {
    return _miny;
  }
  

  /**********************************************************************
   * getMinz() - Retrieve the current value of minz.
   */

  inline double getMinz(void) const
  {
    return _minz;
  }
  

  /**********************************************************************
   * getOriginLat() - Retrieve the current value of the latitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual inline double getOriginLat(void) const
  {
    return 0.0;
  }
  
  
  /**********************************************************************
   * getOriginLon() - Retrieve the current value of the longitude of the
   *                  projection origin.  For projections that don't
   *                  support an origin, 0.0 will be returned.
   */

  virtual inline double getOriginLon(void) const
  {
    return 0.0;
  }
  
  
  /**********************************************************************
   * setOrigin() - Sets the projection origin for the projection, if the
   *               projection uses an origin.  Does nothing for projections
   *               that don't use an origin.
   */

  virtual inline void setOrigin(const double origin_lat,
				const double origin_lon)
  {
    // Do nothing by default
  }
  
  
  /**********************************************************************
   * getRotation() - Retrieve the current value of the projection rotation.
   *                 For projections that don't support a rotation value,
   *                 0.0 will be returned.
   */

  virtual inline double getRotation(void) const
  {
    return 0.0;
  }
  
  
  /**********************************************************************
   * getPole() - Retrieve the current value of the pole (north or south).
   *                 For projections that don't support a pole,
   *                 POLE_NORTH will be returned.
   */

  virtual inline PjgTypes::pole_type_t getPole(void) const
  {
    return PjgTypes::POLE_NORTH;
  }


  /**********************************************************************
   * getLat1() - Retrieve the current value of the projection lat1.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual inline double getLat1(void) const
  {
    return 0.0;
  }
  
  
  /**********************************************************************
   * getLat2() - Retrieve the current value of the projection lat2.  For
   *             projections that don't latitude values in their definitions,
   *             0.0 will be returned.
   */

  virtual inline double getLat2(void) const
  {
    return 0.0;
  }
  
  
  ////////////////////////////
  // Static transformations //
  ////////////////////////////

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
  
  ////////////////////////////
  // Virtual public methods //
  ////////////////////////////

  /**********************************************************************
   * latlon2xy() - Convert the given lat/lon location to the grid location
   *               in grid units.
   */

  virtual void latlon2xy(const double lat, const double lon,
			 double  &x, double &y) const = 0;

  /**********************************************************************
   * xy2latlon() - Convert the given grid location specified in grid units
   *               to the appropriate lat/lon location.
   *
   * Z is only used if set for the polar radar projection.
   */
  
  virtual void xy2latlon(const double x, const double y,
			 double &lat, double &lon,
			 const double z = -9999.0) const = 0;

  /**********************************************************************
   * km2x() - Converts the given distance in kilometers to the same
   *          distance in the units appropriate to the projection.
   */

  virtual double km2x(const double km) const = 0;
  
  /**********************************************************************
   * x2km() - Converts the given distance to kilometers.  The distance
   *          is assumed to be in the units appropriate to the projection.
   */

  virtual double x2km(const double x) const = 0;
  
  /**********************************************************************
   * km2xGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the X axis.
   */

  virtual double km2xGrid(const double x_km) const = 0;
  
  /**********************************************************************
   * km2yGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the Y axis.
   */

  virtual double km2yGrid(const double y_km) const = 0;
  
  /**********************************************************************
   * xGrid2km() - Converts the given distance in number of grid spaces
   *              along the X axis to kilometers.  If y_index is non-negative,
   *              the conversion is done at that point in the grid;
   *              otherwise, the conversion is done at the center of the
   *              grid.
   */

  virtual double xGrid2km(const double x_grid,
			  const int y_index = -1) const = 0;
  
  /**********************************************************************
   * yGrid2km() - Converts the given distance in number of grid spaces
   *              along the Y axis to kilometers.
   */

  virtual double yGrid2km(const double y_grid) const = 0;


  /////////////////////////////////////////////////////////////////
  // The following methods are implemented here using the above  //
  // virtual methods so they do not need to be overridden except //
  // in special cases.                                           //
  /////////////////////////////////////////////////////////////////

  /**********************************************************************
   * latlon2xyIndex() - Computes the the data x, y indices for the given
   *                    lat/lon location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  virtual int latlon2xyIndex(const double lat, const double lon,
			     int &x_index, int &y_index) const;

  /**********************************************************************
   * latlon2arrayIndex() - Computes the index into the data array.
   *
   * Returns 0 on success, -1 on failure (data outside grid).
   */

  virtual int latlon2arrayIndex(const double lat, const double lon,
				int &array_index) const;

  /**********************************************************************
   * xyIndex2latlon() - Computes the lat & lon given ix and iy rel to grid.
   */

  virtual void xyIndex2latlon(const int ix, const int iy,
			      double &lat, double &lon) const;
  

  /**********************************************************************
   * xy2xyIndex() - Computes the the data x, y indices for the given
   *                grid units location.
   *
   * Returns 0 on success, -1 on failure (data outside grid)
   */

  virtual int xy2xyIndex(const double x, const double y,
			 int &x_index, int &y_index) const
  {
    x_index = (int)((x - _minx) / _dx + 0.5);
    y_index = (int)((y - _miny) / _dy + 0.5);
    
    if (x_index < 0 || x_index >= _nx ||
	y_index < 0 || y_index >= _ny)
      return -1;
    
    return 0;
  }
  

  /**********************************************************************
   * xyIndex2arrayIndex() - Computes the index into the data array.
   *
   * Returns the calculated array index on success, -1 on failure
   * (data outside grid).
   */

  virtual int xyIndex2arrayIndex(const int ix, const int iy,
				 const int iz = 0) const;
  

  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * print() - Print the projection parameters to the given stream
   */

  virtual void print(ostream &stream) const;
  

  ///////////////
  // Operators //
  ///////////////

  bool operator==(const PjgCalc &other) const
  {
    if (_nz == 1 && other._nz == 1)
      return (_projType == other._projType &&
	      _nx == other._nx &&
	      _ny == other._ny &&
	      _dx == other._dx &&
	      _dy == other._dy &&
	      _minx == other._minx &&
	      _miny == other._miny);

    return (_projType == other._projType &&
	    _nx == other._nx &&
	    _ny == other._ny &&
	    _nz == other._nz &&
	    _dx == other._dx &&
	    _dy == other._dy &&
	    _dz == other._dz &&
	    _minx == other._minx &&
	    _miny == other._miny &&
	    _minz == other._minz);
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  static const double TINY_DIST;
  static const double TINY_FLOAT;
  
  PjgTypes::proj_type_t _projType;

  bool _isConstantNx;
  vector< int > _nxList;
  
  int _nx;
  int _ny;
  int _nz;
  
  double _dx;
  double _dy;
  double _dz;
  
  double _minx;
  double _miny;
  double _minz;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _latlonPlusRTheta(const double cos_colat1,
			 const double sin_colat1,
			 const double lon1_rad,
			 const double r,
			 const double theta_rad,
			 double &lat2, double &lon2) const;
  
  void _latlon2RTheta(const double colat1,
		      const double cos_colat1,
		      const double sin_colat1,
		      const double lon1,
		      const double lat2,
		      const double lon2,
		      double &r, double &theta_rad) const;
  

  double _km2grid(double km_dist,
		  double grid_delta) const;
  
  double _grid2km(double grid_dist,
		  double grid_delta) const;
  
  ///////////////////////////////
  // Virtual protected methods //
  ///////////////////////////////

private:

};

#endif
