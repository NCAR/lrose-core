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
 * WorldPolygon2D.cc: class implementing a polygon specified by
 *                    WorldPoint2D points.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>
#include <cstdio>
#include <cstring>

#include <euclid/geometry.h>
#include <euclid/Pjg.hh>
#include <euclid/WorldPoint2D.hh>
#include <euclid/WorldPolygon2D.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

WorldPolygon2D::WorldPolygon2D() :
  _polygonGrid(0),
  _minPolygonX(-1),
  _maxPolygonX(-1),
  _minPolygonY(-1),
  _maxPolygonY(-1)
{
}


// Copy constructor

WorldPolygon2D::WorldPolygon2D(const WorldPolygon2D& rhs) :
  _polygonGrid(0),
  _minPolygonX(-1),
  _maxPolygonX(-1),
  _minPolygonY(-1),
  _maxPolygonY(-1)
{
  // Copy the points vector

  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = rhs._points.begin();
       point_iter != rhs._points.end(); ++point_iter)
  {
    WorldPoint2D *point = new WorldPoint2D(**point_iter);
    _points.push_back(point);
  } /* endfor - point_iter */

  // Don't copy the gridded polygon since it isn't used much.
  // If the new object needs it, it can recalculate it.
}


/**********************************************************************
 * Destructor
 */

WorldPolygon2D::~WorldPolygon2D(void)
{
  // Reclaim the space used for the polygon point list

  vector< WorldPoint2D* >::const_iterator point_iter;
  
  for (point_iter = _points.begin();
       point_iter != _points.end();
       ++point_iter)
    delete *point_iter;
  
  _points.erase(_points.begin(), _points.end());
  
  // Delete the polygon grid if it has been constructed

  if (_polygonGrid)
    delete [] _polygonGrid;
}
  

/////////////////////////////////
// Polygon maintenance methods //
/////////////////////////////////

/**********************************************************************
 * addPoint() - Adds the given point to the end of the polygon.  The
 *              pointer to the point is saved as a part of the polygon,
 *              so you must not change the values or delete the point
 *              object after calling this routine.
 */

void WorldPolygon2D::addPoint(WorldPoint2D *point)
{
  // Delete the polygon grid since it will no longer be valid

  delete [] _polygonGrid;
  _polygonGrid = 0;

  // Now add the point to the polygon

  _points.push_back(point);
}


void WorldPolygon2D::addPoint(const double lat, const double lon)
{
  WorldPoint2D *point = new WorldPoint2D(lat, lon);
  addPoint(point);
}

  

/**********************************************************************
 * growKm() - Increases the size of the polygon all around the perimeter
 *            by the given length in kilometers.
 */

void WorldPolygon2D::growKm(const double growth_km)
{
  // If the growth value is 0, nothing should happen

  if (growth_km == 0.0)
    return;
  
  // Delete the polygon grid since it will no longer be valid

  delete [] _polygonGrid;
  _polygonGrid = 0;

  // Get the centroid for the polygon

  WorldPoint2D centroid = calcCentroid();
  
  // Now, loop through all of the points in the polygon and increase
  // the point's distance from the centroid by the length given.

  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = _points.begin(); point_iter != _points.end();
       ++point_iter)
  {
    WorldPoint2D *point = *point_iter;
    
    double r, theta;
    
    Pjg::latlon2RTheta(centroid.lat, centroid.lon,
		       point->lat, point->lon,
		       r, theta);
    
    Pjg::latlonPlusRTheta(centroid.lat, centroid.lon,
			  r + growth_km, theta,
			  point->lat, point->lon);
    
  } /* endfor -- point_iter */
  
}
  

///////////////////////////////
// Polygon iteration methods //
///////////////////////////////

/**********************************************************************
 * getFirstPoint() - Retrieve the first point from the polygon.  Returns
 *                   NULL if there are no points in the polygon.
 */

WorldPoint2D *WorldPolygon2D::getFirstPoint(void) const
{
  _pointsIterator = _points.begin();
  
  if (_pointsIterator == _points.end())
    return (WorldPoint2D *)NULL;
  
  return *_pointsIterator;
}
  

/**********************************************************************
 * getNextPoint() - Retrieve the next point from the polygon.  Returns
 *                  NULL if there are no more points in the polygon.
 *
 * Note that getFirstPoint() MUST be called before using this method.
 */

WorldPoint2D *WorldPolygon2D::getNextPoint(void) const
{
  _pointsIterator++;
  
  if (_pointsIterator == _points.end())
    return (WorldPoint2D *)NULL;
  
  return *_pointsIterator;
}
  

/////////////////////////////////
// Polygon calculation methods //
/////////////////////////////////

/**********************************************************************
 * calcCentroid() - Calculate the centroid of the polygon.
 */

WorldPoint2D WorldPolygon2D::calcCentroid(void)
{
  // Check for empty polygon

  if (_points.size() <= 2)
    return WorldPoint2D(0.0, 0.0);
  
  // The centroid is the average of the lat/lon values for the vertices

  double total_lat = 0.0;
  double total_lon = 0.0;
  int num_points = 0;
  
  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = _points.begin();
       point_iter != _points.end();
       ++point_iter)
  {
    WorldPoint2D *point = *point_iter;
    
    total_lat += point->lat;
    total_lon += point->lon;
    ++num_points;
  } /* endfor - point_iter */
  
  return WorldPoint2D(total_lat / (double)num_points,
		      total_lon / (double)num_points);
}


/**********************************************************************
 * extrapolate() - Extrapolate the polyline as specified.
 */

void WorldPolygon2D::extrapolate(const double distance_km,
				 const double direction_rad)

{
  // Extrapolate each of the points in the polygon

  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = _points.begin(); point_iter != _points.end(); ++point_iter)
  {
    WorldPoint2D *point = *point_iter;
    
    double new_lat, new_lon;
    
    Pjg::latlonPlusRTheta(point->lat, point->lon, distance_km, direction_rad,
			  new_lat, new_lon);
    
    point->setPoint(new_lat, new_lon);
  } /* endfor - point */
  
  // Clear out the gridded polygon since it is no longer valid

  delete [] _polygonGrid;
  _polygonGrid = 0;

  _minPolygonX = -1;
  _maxPolygonX = -1;
  _minPolygonY = -1;
  _maxPolygonY = -1;
  
}

/**********************************************************************
 * getGridMax() - Get the maximum data value from the given grid within
 *                this polygon.
 *
 * Returns the maximum data value found, or missing_data_value if no
 * data values were found.
 */

double WorldPolygon2D::getGridMax(const Pjg &projection,
				  const double missing_data_value,
				  const double bad_data_value,
				  const fl32 *data_grid) const
{
  _getGriddedPolygon(projection);
						   
  // Find the maximum interest value within the grid

  double max_data_value = missing_data_value;
  
  for (int x = _minPolygonX; x <= _maxPolygonX; ++x)
  {
    for (int y = _minPolygonY; y <= _maxPolygonY; ++y)
    {
      int data_index = projection.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[data_index])
      {
	double data_value = data_grid[data_index];
	
	if (data_value != missing_data_value &&
	    data_value != bad_data_value)
	{
	  if (max_data_value == missing_data_value ||
	      max_data_value < data_value)
	    max_data_value = data_value;
	}
      }

    } /* endfor - y */
  } /* endfor - x */
  
  return max_data_value;
}

/**********************************************************************
 * getGridMin() - Get the minimum data value from the given grid within
 *                this polygon.
 *
 * Returns the minimum data value found, or missing_data_value if no
 * data values were found.
 */

double WorldPolygon2D::getGridMin(const Pjg &projection,
				  const double missing_data_value,
				  const double bad_data_value,
				  const fl32 *data_grid) const
{
  _getGriddedPolygon(projection);
						   
  // Find the maximum interest value within the grid

  double min_data_value = missing_data_value;
  
  for (int x = _minPolygonX; x <= _maxPolygonX; ++x)
  {
    for (int y = _minPolygonY; y <= _maxPolygonY; ++y)
    {
      int data_index = projection.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[data_index])
      {
	double data_value = data_grid[data_index];
	
	if (data_value != missing_data_value &&
	    data_value != bad_data_value)
	{
	  if (min_data_value == missing_data_value ||
	      min_data_value > data_value)
	    min_data_value = data_value;
	}
      }

    } /* endfor - y */
  } /* endfor - x */
  
  return min_data_value;
}

/**********************************************************************
 * getGridAvg() - Get the Average data value from the given grid within
 *                this polygon.
 *
 * Returns the average data value found, or missing_data_value if no
 * data values were found.
 */

double WorldPolygon2D::getGridAvg(const Pjg &projection,
                                  const double missing_data_value,
                                  const double bad_data_value,
                                  const fl32 *data_grid) const {
  _getGriddedPolygon(projection);

  // Find the average interest value within the grid

  double sum = missing_data_value;
  int num_values = 0;

  for (int x = _minPolygonX; x <= _maxPolygonX; ++x) {
    for (int y = _minPolygonY; y <= _maxPolygonY; ++y) {
      int data_index = projection.xyIndex2arrayIndex(x, y);

      if (_polygonGrid[data_index]) {
        double data_value = data_grid[data_index];

        if (data_value != missing_data_value &&
            data_value != bad_data_value) {
          num_values = num_values + 1;
          sum = sum + data_value;
        }
      }

    } /* endfor - y */
  } /* endfor - x */

  return (sum / num_values);
}

/**********************************************************************
 * getGridNumValues() - Get the number of grid squares within this polygon
 *                      with the given data value.
 *
 * Returns the number of grid points found.
 */

size_t WorldPolygon2D::getGridNumValues(const Pjg &projection,
					const double data_value,
					const fl32 *data_grid) const
{
  _getGriddedPolygon(projection);
						   
  // Find the maximum interest value within the grid

  int num_values = 0;
  
  for (int x = _minPolygonX; x <= _maxPolygonX; ++x)
  {
    for (int y = _minPolygonY; y <= _maxPolygonY; ++y)
    {
      int data_index = projection.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[data_index])
      {
	double grid_data_value = data_grid[data_index];
	
	if (grid_data_value == data_value)
	  ++num_values;
      }

    } /* endfor - y */
  } /* endfor - x */
  
  return num_values;
}


/**********************************************************************
 * getGridSize() - Get the number of grid squares within this polygon.
 *
 * Returns the number of grid squares.
 */

size_t WorldPolygon2D::getGridSize(const Pjg &projection) const
{
  _getGriddedPolygon(projection);
						   
  // Find the maximum interest value within the grid

  int num_squares = 0;
  
  for (int x = _minPolygonX; x <= _maxPolygonX; ++x)
  {
    for (int y = _minPolygonY; y <= _maxPolygonY; ++y)
    {
      int data_index = projection.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[data_index])
	++num_squares;

    } /* endfor - y */
  } /* endfor - x */
  
  return num_squares;
}


/**********************************************************************
 * inPolyline() - Determine if the given point falls within the polyline
 *                when it is gridded.
 *
 * Returns the true if the point lies within the polyline, false otherwise.
 */

bool WorldPolygon2D::inPolyline(const Pjg &projection,
				const double lat, const double lon) const
{
  // Make sure we have the gridded polyline

  _getGriddedPolygon(projection);

  // Find the point in the grid.  If the point isn't in the grid, then it
  // isn't in the polyline

  int index;
  
  if (projection.latlon2arrayIndex(lat, lon, index) < 0)
    return false;
  
  return _polygonGrid[index];
}


///////////////
// Operators //
///////////////

WorldPolygon2D& WorldPolygon2D::operator= (const WorldPolygon2D &rhs)
{
  // Copy the points

  vector< WorldPoint2D* >::const_iterator point;
  
  for (point = rhs._points.begin(); point != rhs._points.end(); ++point)
  {
    WorldPoint2D *new_point = new WorldPoint2D(*point);
    _points.push_back(new_point);
  }
  
  _pointsIterator = _points.begin();
  
  _polygonProjection = rhs._polygonProjection;
  
  // For now, make the user recalculate the polygon if needed.  Could allocate
  // space and copy the polygon later if needed.

  _polygonGrid = 0;
  
  _minPolygonX = -1;
  _maxPolygonX = -1;
  _minPolygonY = -1;
  _maxPolygonY = -1;
  
  return *this;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

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

void WorldPolygon2D::_getGriddedPolygon(const Pjg &projection) const
{
  // See if we already have the appropriate gridded polygon.  If
  // this is the case, we don't need to do anything.

  if (_polygonGrid != 0 &&
      projection == _polygonProjection)
    return;
  
  // Reinitialize all of the polygon information

  _polygonProjection = projection;
  delete [] _polygonGrid;
  _polygonGrid = 0;
  
  // Convert the polygon to the format needed for the
  // fill_polygon function.

  Point_d *vertices = new Point_d[_points.size() + 1];
  int num_vert = 0;

  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = _points.begin();
       point_iter != _points.end();
       ++point_iter)
  {
    WorldPoint2D *point = *point_iter;
    
    // Convert the lat/lon values for the point to X, Y indices

    int x, y;
    double lat_point, lon_point;

    // Check if longitude point falls inside the grid.
    // If not, move point to edge of the grid.

    if ( point->lon <= projection.getMinx() )
    {
      lon_point = projection.getMinx();
    }
    else if ( point->lon >= projection.getMinx() + projection.getDx() * projection.getNx() )
    {
      lon_point = projection.getMinx() + projection.getDx() * projection.getNx();
    }
    else
    {
      lon_point = point->lon;
    }
  
    // Check if latitude point falls inside the grid.
    // If not, move point to edge of the grid.

    if ( point->lat <= projection.getMiny() )
    {
      lat_point = projection.getMiny();
    }
    else if ( point->lat >= projection.getMiny() + projection.getDy() * projection.getNy() )
    {
      lat_point = projection.getMiny() + projection.getDy() * projection.getNy();
    }
    else
    {
      lat_point = point->lat;
    }
    
    projection.latlon2xyIndex(lat_point, lon_point, x, y);

    // Don't put duplicate points in the polyline.  This will happen if our
    // grid is coarse compared to our polygon point spacing.

    if (num_vert > 0 &&
	x == vertices[num_vert-1].x && y == vertices[num_vert-1].y)
      continue;

    if (x < 0)
      vertices[num_vert].x = 0.0;
    else if (x >= projection.getNx())
      vertices[num_vert].x = (double)(projection.getNx() - 1);
    else
      vertices[num_vert].x = (double)x;

    if (y < 0)
      vertices[num_vert].y = 0.0;
    else if (y >= projection.getNy())
      vertices[num_vert].y = (double)(projection.getNy() - 1);
    else
      vertices[num_vert].y = (double)y;
    
    if (num_vert == 0 || _minPolygonX > (int)vertices[num_vert].x)
      _minPolygonX = (int)vertices[num_vert].x;
    if (num_vert == 0 || _maxPolygonX < (int)vertices[num_vert].x)
      _maxPolygonX = (int)vertices[num_vert].x;
    if (num_vert == 0 || _minPolygonY > (int)vertices[num_vert].y)
      _minPolygonY = (int)vertices[num_vert].y;
    if (num_vert == 0 || _maxPolygonY < (int)vertices[num_vert].y)
      _maxPolygonY = (int)vertices[num_vert].y;
    
    // Note that we have to increment num_vert here and not within the for
    // statement because we only want to increment it if we actually get
    // here, not if we continue earlier.

    ++num_vert;
    
  } /* endfor - point */
  
  // Make sure that the last point closes the polyline

  if (vertices[num_vert-1].x != vertices[0].x ||
      vertices[num_vert-1].y != vertices[0].y)
  {
    vertices[num_vert].x = vertices[0].x;
    vertices[num_vert].y = vertices[0].y;
    ++num_vert;
  }
  
  // Get the gridded filled polygon

  int grid_size = projection.getNx() * projection.getNy();

  _polygonGrid = new unsigned char[grid_size];
  memset(_polygonGrid, 0, grid_size * sizeof(unsigned char));

  // If all the vertices are on one edge of the grid then
  // don't fill this polygon. This means the plygon originally 
  // hand no pts inside the grid.
  
  if(_minPolygonX == _maxPolygonX ||
     _minPolygonY == _maxPolygonY)
  {
    delete [] vertices;
    return;
  }

  int num_poly_pts = EG_fill_polygon(vertices,
				     num_vert,
				     projection.getNx(),
				     projection.getNy(),
				     0.0, 0.0,
				     1.0, 1.0,
				     _polygonGrid,
				     1);

  delete [] vertices;
  
  // If this is a small polygon and we are using a coarse grid,
  // sometimes the polygon won't have any grid points.  In this
  // case, set the grid square that contains the polygon on so
  // that we have something.

  if (num_poly_pts == 0)
    _polygonGrid[_minPolygonX + (_minPolygonY * projection.getNx())] = 1;

}


/**********************************************************************
 *              Friends                                               *
 **********************************************************************/

ostream& operator<< (ostream &os, const WorldPolygon2D &polygon)
{
  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = polygon._points.begin();
       point_iter != polygon._points.end(); ++point_iter)
    os << *point_iter << endl;
  
  return os;
}

ostream& operator<< (ostream &os, const WorldPolygon2D *polygon)
{
  vector< WorldPoint2D* >::iterator point_iter;
  
  for (point_iter = polygon->_points.begin();
       point_iter != polygon->_points.end(); ++point_iter)
    os << *point_iter << endl;
  
  return os;
}
