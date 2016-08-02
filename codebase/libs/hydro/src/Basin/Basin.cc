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
///////////////////////////////////////////////////////////////
// Basin.cc
//
// Class representing a watershed basin.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <strings.h>
#include <vector>

#include <toolsa/os_config.h>
#include <euclid/geometry.h>
#include <euclid/point.h>
#include <euclid/WorldPoint2D.hh>
#include <hydro/Basin.hh>
#include <hydro/BasinField.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <shapelib/shapefil.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
using namespace std;


/*********************************************************************
 * Constructors
 */

Basin::Basin(const bool debug_flag) :
  _debugFlag(debug_flag),
  _infoLoaded(false),
  _id(-1),
  _minLat(0.0),
  _minLon(0.0),
  _maxLat(0.0),
  _maxLon(0.0),
  _euclidPerimeter(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

Basin::~Basin()
{
  // Free the dynamically allocated space

  delete _euclidPerimeter;
}


/*********************************************************************
 * createMask() - Create a gridded mask representing the position of
 *                this basin on the given projection.
 *
 * If successful, allocates space for the gridded mask and returns
 * a pointer to that mask.  The pointer must be freed by the caller
 * using ufree().  The grid will contain 0's for grid squares outside
 * of the basin and non-0 values for squares within the basin.
 *
 * If not successful, returns 0.
 */

unsigned char *Basin::createMask(const MdvxProj &projection,
				 int &min_x, int &max_x,
				 int &min_y, int &max_y) const
{
  // Get the projection information

  Mdvx::coord_t proj_params = projection.getCoord();
  
  // Initialize the return values

  min_x = 0;
  max_x = proj_params.nx - 1;
  min_y = 0;
  max_y = proj_params.ny - 1;
  
  // Create and initialize the mask grid

  unsigned char *mask =
    (unsigned char *)umalloc(proj_params.nx * proj_params.ny *
			     sizeof(unsigned char));
  memset(mask, 0, proj_params.nx * proj_params.ny * sizeof(unsigned char));
  
  // Allocate space for the array of vertices used in the euclid
  // routine to create the mask grid.

  Point_d *vertices =
    (Point_d *)umalloc(_perimeter.size() * sizeof(Point_d));
  
  // Convert the lat/lon polygon to the format needed for the
  // fill_polygon routine.  Here we are using X/Y index values for
  // the points so we can't use _euclidPerimiter.
  // Get the bounding box while we're here

  vector< WorldPoint2D >::const_iterator pt_iter;
  int i;
  
  for (pt_iter = _perimeter.begin(), i = 0;
       pt_iter != _perimeter.end();
       ++pt_iter, ++i)
  {
    WorldPoint2D point = *pt_iter;
    int x, y;
    
    projection.latlon2xyIndex(point.lat, point.lon,
			      x, y);
    
    if (_debugFlag)
      cerr << point.lat << ", " << point.lon << " ---> " <<
	x << ", " << y << endl;
    
    if (x < 0)
      vertices[i].x = 0.0;
    else if (x >= proj_params.nx)
      vertices[i].x = (double)(proj_params.nx - 1);
    else
      vertices[i].x = (double)x;
    
    if (y < 0)
      vertices[i].y = 0.0;
    else if (y >= proj_params.ny)
      vertices[i].y = (double)(proj_params.ny - 1);
    else
      vertices[i].y = (double)y;
    
    if (i == 0)
    {
      min_x = x;
      max_x = x;
      min_y = y;
      max_y = y;
    }
    else
    {
      if (min_x > x)
	min_x = x;
      if (max_x < x)
	max_x = x;
      if (min_y > y)
	min_y = y;
      if (max_y < y)
	max_y = y;
    }
      
  } /* endfor - i */
    
  if (_debugFlag)
  {
    cerr << "Grid: min_x = " << min_x << ", max_x = " << max_x <<
      ", min_y = " << min_y << ", max_y = " << max_y << endl;
    cerr << "      grid_nx = " << proj_params.nx <<
      ", grid_ny = " << proj_params.ny << endl;
  }
      
  // Get the gridded filled polygon.

//  long points_filled =
    EG_fill_polygon(vertices,
		    _perimeter.size(),
		    proj_params.nx, proj_params.ny,
		    0.0, 0.0,
		    1.0, 1.0,
		    mask,
		    1);
      
  ufree(vertices);
    
  if (_debugFlag)
  {
    cerr << "Mask grid:" << endl;
    cerr << "----------" << endl;
    
    for (int y = 0; y < proj_params.ny; ++y)
    {
      for (int x = 0; x < proj_params.nx; ++x)
	fprintf(stderr, "%d", mask[x + (y * proj_params.nx)]);

      cerr << endl;
      
    } /* endfor - y */
  }
  
  return mask;
}


/*********************************************************************
 * getIdFieldFromShapeBase() - Determine the basin ID field name from
 *                             the shape file base.
 */

string Basin::getIdFieldFromShapeBase(const string shape_base)
{
  char id_field[BUFSIZ];
  
  STRcopy(id_field, shape_base.c_str(), BUFSIZ);
  
  char *slash_pos = rindex(id_field, '/');
  char *id_field_begin;
  
  if (slash_pos == 0)
    id_field_begin = id_field;
  else
    id_field_begin = slash_pos + 1;
  
  STRconcat(id_field_begin, "_", BUFSIZ);
  
  for (int i = 0; i < (int)strlen(id_field_begin); ++i)
    id_field_begin[i] = toupper(id_field_begin[i]);
  
  string id_field_string = id_field_begin;
  
  return id_field_string;
}


/*********************************************************************
 * loadShapeInfo() - Load the basin information from the given shape file.
 *
 * Returns true if the basin information was successfully loaded, false
 * otherwise.
 */

bool Basin::loadShapeInfo(const string shape_file_base,
			  const int shape_number,
			  const string basin_id_field)
{
  const string routine_name = "loadShapeInfo()";
  
  // Open the shape file

  string shape_file = shape_file_base + ".shp";
  SHPHandle shape_handle = SHPOpen(shape_file.c_str(), "rb");
  
  if (shape_handle == 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error opening shape file: " << shape_file << endl;
    
    return false;
  }
  
  // Open the database file

  string dbf_file = shape_file_base + ".dbf";
  DBFHandle dbf_handle = DBFOpen(dbf_file.c_str(), "rb");
  
  if (dbf_handle == 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error opening database file: " << dbf_file << endl;
    
    SHPClose(shape_handle);
    
    return false;
  }
  
  // Construct a basin id field name, if one wasn't provided

  string actual_basin_id_field;
  
  if (basin_id_field.empty())
    actual_basin_id_field = getIdFieldFromShapeBase(shape_file_base);
  else
    actual_basin_id_field = basin_id_field;
  
  // Load the shape information

  bool return_code = loadShapeInfo(shape_handle, dbf_handle,
				   shape_number,
				   actual_basin_id_field);
  
  // Close the shape files since we don't need them anymore

  SHPClose(shape_handle);
  DBFClose(dbf_handle);
  
  return return_code;
}


bool Basin::loadShapeInfo(const SHPHandle shape_handle,
			  const DBFHandle dbf_handle,
			  const int shape_number,
			  const string basin_id_field)
{
  const string routine_name = "loadShapeInfo()";
  
  // Make sure an id field name was supplied

  if (basin_id_field.empty())
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "basin_id_field must not be empty" << endl;
    cerr << "*** Skipping basin ***" << endl;
    
    return false;
  }
  
  // Read the shape information for the basin

  SHPObject *basin_shape;
  
  if ((basin_shape = SHPReadObject(shape_handle, shape_number)) == 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error reading shape " << shape_number << " from shape file" <<
      endl;
    cerr << "*** Skipping basin ***" << endl;
    
    return false;
  }
  
  // Allocate space for the perimeter array used for the euclid routines

  delete _euclidPerimeter;
  
  _euclidPerimeter = new Point_d[basin_shape->nVertices];
  
  // Save the basin perimeter

  for (int vertex_num = 0; vertex_num < basin_shape->nVertices; ++vertex_num)
  {
    // Update the vertex vector

    WorldPoint2D vertex(basin_shape->padfY[vertex_num],
			basin_shape->padfX[vertex_num]);
    
    _perimeter.push_back(vertex);

    // Update the vertex array for the euclid library

    _euclidPerimeter[vertex_num].x = basin_shape->padfX[vertex_num];
    _euclidPerimeter[vertex_num].y = basin_shape->padfY[vertex_num];
    
  } /* endfor - vertex_num */
  
  // Save the basin perimeter limits

  _minLat = basin_shape->dfYMin;
  _minLon = basin_shape->dfXMin;
  _maxLat = basin_shape->dfYMax;
  _maxLon = basin_shape->dfXMax;
  
  // Free the memory used for the basin shape information

  SHPDestroyObject(basin_shape);
  
  // Now read in the database information for the basin.  Look for the
  // ID field while we're here.

  int field_count = DBFGetFieldCount(dbf_handle);
  
  for (int i = 0; i < field_count; ++i)
  {
    BasinField field(dbf_handle, shape_number, i, _debugFlag);
    
    _dbfInfo.push_back(field);

    // See if this is the ID field.

    if (field.getTitle() == basin_id_field)
    {
      if (field.getFieldType() != BasinField::TYPE_INTEGER)
      {
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "Id field must contain an integer value" << endl;
	cerr << "*** Skipping basin ***" << endl;
	
	return false;
      }
      
      _id = field.getIntegerValue();
    }
  }
  
  // Make sure an ID field was found

  if (_id < 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "No ID value found in shape file" << endl;
    cerr << "*** Skipping basin ***" << endl;
    
    return false;
  }
  
  // Return, indicating that the information was successfully loaded.

  _infoLoaded = true;
  
  return true;
}


/*********************************************************************
 * pointInBasin() - Determines whether the given point lies within the
 *                  basin.
 *
 * Returns true if the point is within the basin, false otherwise.
 */

bool Basin::pointInBasin(const double lat, const double lon) const
{
  Point_d point;
  
  point.x = lon;
  point.y = lat;
  
  return EG_point_in_polygon(point,
			     _euclidPerimeter,
			     _perimeter.size());
}


/*********************************************************************
 * print() - Print the current basin information to the given stream
 *           for debugging purposes.
 */

void Basin::print(ostream &stream, bool print_vertices) const
{
  stream << "Basin information:" << endl;
  stream << "==================" << endl;
  stream << "debug flag = " << _debugFlag << endl;
  stream << "info loaded flag = " << _infoLoaded << endl;
  stream << "id = " << _id << endl;
  stream << endl;
  stream << "Perimeter information:" << endl;
  stream << "----------------------" << endl;
  stream << "min lat = " << _minLat << endl;
  stream << "min lon = " << _minLon << endl;
  stream << "max lat = " << _maxLat << endl;
  stream << "max lon = " << _maxLon << endl;
  stream << endl;
  stream << "Database information:" << endl;
  stream << "---------------------" << endl;
  
  vector< BasinField >::const_iterator dbf_iter;
  
  for (dbf_iter = _dbfInfo.begin();
       dbf_iter != _dbfInfo.end();
       ++dbf_iter)
  {
    dbf_iter->print(cerr);
    cerr << endl;
  }
  
  stream << endl;
  
  if (print_vertices)
  {
    vector< WorldPoint2D >::const_iterator vert_iter;

    stream << endl;
    stream << "Vertices:" << endl;
    stream << "---------" << endl;

    for (vert_iter = _perimeter.begin();
	 vert_iter != _perimeter.end();
	 ++vert_iter)
      stream << "  " << vert_iter->lat << ", " << vert_iter->lon << endl;
  }
  
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/
