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
 * WorldPolygon2D.hh: class implementing a polygon specified by
 *                    WorldPoint2D points.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef WorldPolygon2D_HH
#define WorldPolygon2D_HH

#include <cstdio>
#include <vector>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <euclid/WorldPoint2D.hh>

using namespace std;


class WorldPolygon2D
{
 public:

  // Constructors

  WorldPolygon2D();
  WorldPolygon2D(const WorldPolygon2D& rhs);
  
  // Destructor

  ~WorldPolygon2D(void);
  
  /////////////////////////////////
  // Polygon maintenance methods //
  /////////////////////////////////

  // Adds the given point to the end of the polygon.  The pointer to the
  // point is saved as a part of the polygon, so you must not change the
  // values or delete the point object after calling this routine.

  void addPoint(WorldPoint2D *point);
  void addPoint(const double lat, const double lon);
  
  /**********************************************************************
   * growKm() - Increases the size of the polygon all around the perimeter
   *            by the given length in kilometers.
   */

  void growKm(const double growth_km);
  

  /////////////////////////////////
  // Polygon calculation methods //
  /////////////////////////////////

  /**********************************************************************
   * calcCentroid() - Calculate the centroid of the polygon.
   */

  WorldPoint2D calcCentroid(void);
  

  /**********************************************************************
   * extrapolate() - Extrapolate the polyline as specified.
   */

  void extrapolate(const double distance_km,
		   const double direction_rad);
  

  /**********************************************************************
   * inPolyline() - Determine if the given point falls within the polyline
   *                when it is gridded.
   *
   * Returns the true if the point lies within the polyline, false otherwise.
   */

  bool inPolyline(const Pjg &projection,
		  const double lat, const double lon) const;
  

  /**********************************************************************
   * getGridMax() - Get the maximum data value from the given grid within
   *                this polygon.
   *
   * Returns the maximum data value found, or missing_data_value if no
   * data values were found.
   */

  double getGridMax(const Pjg &projection,
		    const double missing_data_value,
		    const double bad_data_value,
		    const fl32 *data_grid) const;

  /**********************************************************************
   * getGridMin() - Get the minimum data value from the given grid within
   *                this polygon.
   *
   * Returns the minimum data value found, or missing_data_value if no
   * data values were found.
   */

  double getGridMin(const Pjg &projection,
		    const double missing_data_value,
		    const double bad_data_value,
		    const fl32 *data_grid) const;

  /**********************************************************************
   * getGridAvg() - Get the average data value from the given grid within
   *                this polygon.
   *
   * Returns the average data value found, or missing_data_value if no
   * data values were found.
   */

  double getGridAvg(const Pjg &projection,
                    const double missing_data_value,
                    const double bad_data_value,
                    const fl32 *data_grid) const;

  /**********************************************************************
   * getGridNumValues() - Get the number of grid squares within this polygon
   *                      with the given data value.
   *
   * Returns the number of grid points found.
   */

  size_t getGridNumValues(const Pjg &projection,
			  const double data_value,
			  const fl32 *data_grid) const;
  

  /**********************************************************************
   * getGridSize() - Get the number of grid squares within this polygon.
   *
   * Returns the number of grid squares.
   */

  size_t getGridSize(const Pjg &projection) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Iterate through the points in the polygon.  These routines
  // return NULL if there is no point to return.  To iterate through the
  // points list, do something like the following:
  //
  //   for (WorldPoint2D *point = polygon.getFirstPoint();
  //        point != (WorldPoint2D *)NULL;
  //        point = polygon.getNextPoint())
  //              ...

  WorldPoint2D *getFirstPoint(void) const;
  WorldPoint2D *getNextPoint(void) const;
  
  // Retrieve the number of points in the polygon

  int getNumPoints(void) const
  {
    return _points.size();
  }
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  friend ostream& operator<< (ostream&, const WorldPolygon2D*);
  friend ostream& operator<< (ostream&, const WorldPolygon2D&);

  
  ///////////////
  // Operators //
  ///////////////

  WorldPolygon2D& operator= (const WorldPolygon2D &rhs);


 protected:

  // The points in the polygon

  mutable vector< WorldPoint2D* > _points;
  
  // The points list iterator, manipulated using getFirstPoint()
  // and getNextPoint()

  mutable vector< WorldPoint2D* >::iterator _pointsIterator;
  
  // The gridded version of the polygon.  If this pointer is set
  // to 0, the grid hasn't been constructed yet and will be constructed
  // by a call to _getGriddedPolygon().

  mutable Pjg _polygonProjection;
  mutable unsigned char *_polygonGrid;
  
  mutable int _minPolygonX;
  mutable int _maxPolygonX;
  mutable int _minPolygonY;
  mutable int _maxPolygonY;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _getGriddedPolygon() - Convert the given polygon to a grid on the
   *                        given projection.
   *
   * Constructs a grid matching the given projection with non-zero entries
   * in grid spaces within the polygon and saves the grid in the _polygonGrid
   * private member.
   *
   * Also fills _minPolygonX, _maxPolygonX, _minPolygonY and _maxPolygonY
   * members with the minimum and maximum X/Y indices of the polygon in
   * the returned grid.
   */

  void _getGriddedPolygon(const Pjg &projection) const;
  

};


#endif
