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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:36:50 $
//   $Id: Shape2Map.cc,v 1.8 2016/03/07 18:36:50 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Shape2Map.cc: Shape2Map program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>

#include <rapformats/Map.hh>
#include <rapformats/MapObject.hh>
#include <rapformats/MapPolyline.hh>
#include <rapformats/MapPoint.hh>
#include <rapformats/MapIcon.hh>
#include <rapformats/MapIconPoint.hh>
#include <shapelib/shapefil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "Shape2Map.hh"
using namespace std;


// Global variables

Shape2Map *Shape2Map::_instance = (Shape2Map *)NULL;



/*********************************************************************
 * Constructor
 */

Shape2Map::Shape2Map(int argc, char **argv)
{
  const string method_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Shape2Map *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

}


/*********************************************************************
 * Destructor
 */

Shape2Map::~Shape2Map()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Shape2Map *Shape2Map::Inst(int argc, char **argv)
{
  if (_instance == (Shape2Map *)NULL)
    new Shape2Map(argc, argv);
  
  return(_instance);
}

Shape2Map *Shape2Map::Inst()
{
  assert(_instance != (Shape2Map *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void Shape2Map::run()
{
  static const string method_name = "Shape2Map::run()";

  // temporary name for point objects, eventually read from .dbf file
  const string point_shape_name ="points";
  
  // Create the object for storing the map information

  Map map;
  
  // Retrieve the shape file base.

  string shape_file_base = _args->getShapeFileBase();
  
  cerr << "*** Converting shape files " << shape_file_base <<
    " to RAP map file format." << endl;
  
  // Open the shape file

  string shape_file_name = shape_file_base + ".shp";
  
  if ((_shapeHandle = SHPOpen(shape_file_name.c_str(), "rb")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening shape file: " << shape_file_name << endl;
    
    return;
  }
  
  // Open the associated database file

  string database_file_name = shape_file_base + ".dbf";
  

  if ((_databaseHandle = DBFOpen(database_file_name.c_str(), "rb")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening database file: " << database_file_name << endl;
    
    return;
  }
  
  // Retrieve the shape information from the file

  int n_shapes;
  int n_shape_types;
  double file_min_bounds[4];
  double file_max_bounds[4];
  
  SHPGetInfo(_shapeHandle, &n_shapes, &n_shape_types,
	     file_min_bounds, file_max_bounds);
  cerr << "   Shape file contains " << n_shapes << " objects" << endl;
  
  // Create an icon definition object, if one is defined in the parameter
  // file.

  MapIconDef *icon_def = 0;
  
  if (_params->icon_points_n > 0)
  {
    vector< MapPointOffset > point_list;
    
    for (int i = 0; i < _params->icon_points_n; ++i)
    {
      MapPointOffset point(_params->_icon_points[i].x_offset,
			   _params->_icon_points[i].y_offset);
      
      point_list.push_back(point);
    } /* endfor - i */
    
    icon_def = new MapIconDef("SHAPEICON", point_list);
    
    map.addObject((MapObject *)icon_def);
    
  } /* endif - _params->icon_points_n > 0 */
  
  // Process each shape in the file

  for (int i = 0; i < n_shapes; ++i)
  {
    // Read the shape from the shape file

    SHPObject *shape;
    
    if ((shape = SHPReadObject(_shapeHandle, i)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading shape number " << i <<
	" from shape file " << shape_file_name << endl;
      cerr << "*** Skipping this shape ***" << endl;
      cerr << endl;
      
      continue;
    }
    
    // Create a map object for the shape, if we can

    MapObject *map_object = 0;

    switch (shape->nSHPType)
    {
    case SHPT_POLYGON :
    case SHPT_ARC :
      map_object = _createPolygonObject(*shape);
      break;

    case SHPT_POINT :
      // Make sure an icon was defined.

      if (icon_def == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Point object found in shape file, but no icon defined" << endl;
	cerr << "You need to define an icon in the Shape2Map parameter file" << endl;
	cerr << "*** Skipping this shape ***" << endl;
	cerr << endl;

	continue;
      }
      
      // Create the map object

      map_object = _createIconObject(*shape, icon_def, i);
      break;
      
    case SHPT_NULL :
    case SHPT_MULTIPOINT :
    case SHPT_POINTZ :
    case SHPT_ARCZ :
    case SHPT_POLYGONZ :
    case SHPT_MULTIPOINTZ :
    case SHPT_POINTM :
    case SHPT_ARCM :
    case SHPT_POLYGONM :
    case SHPT_MULTIPOINTM :
    case SHPT_MULTIPATCH :
      cerr << "WARNING: " << method_name << endl;
      cerr << "Don't know how to convert " << SHPTypeName(shape->nSHPType) <<
	" (" << shape->nSHPType << ") type shape objects to map format" << endl;
      cerr << "*** Skipping shape ***" << endl;
      cerr << endl;
      break;
      
    default:
      cerr << "ERROR: " << method_name << endl;
      cerr << "Unrecognized shape type " << shape->nSHPType <<
	" found in shape file" << endl;
      cerr << "*** Skipping shape ***" << endl;
      cerr << endl;
      break;
    } /* endswitch - shape->nSHPType */
    
    // Add the map object to the map

    if (map_object != 0)
      map.addObject(map_object);
    
    // Destroy the shape object

    SHPDestroyObject(shape);
    
  } /* endfor - i */
  
  // Close the shape files since we're done with them

  SHPClose(_shapeHandle);
  DBFClose(_databaseHandle);
  
  // Write out the map file

  string map_file_name = shape_file_base + ".map";
  FILE *map_file;
  
  if ((map_file = fopen(map_file_name.c_str(), "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening map file " << map_file_name <<
      " for write" << endl;
    return;
  }
  
  if (_params->debug)
  {
    cerr << "Map information:" << endl;
    cerr << "================" << endl;
    
    map.write(stderr);
  }
  
  map.write(map_file);
  
  fclose(map_file);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createIconObject() - Create an icon map object from a point shape
 *                       object.
 */

MapObject *Shape2Map::_createIconObject(const SHPObject &shape,
					MapIconDef *icon_def,
					const int shape_num) const
{
  MapPoint location(shape.padfY[0], shape.padfX[0]);
  MapIcon *icon;
  
  if (_params->include_icon_label)
  {
    // Get the label attribute from the file

    const char *label = DBFReadStringAttribute(_databaseHandle,
					       shape_num,
					       _params->label_field_index);
    
    if (strlen(label) <= 0)
      return 0;
    
    cerr << "Read label attribute <" << label << ">" << endl;
    
    icon = new MapIcon(icon_def, location, label,
		       _params->label_offset_x, _params->label_offset_y);
  }
  else
  {
    icon = new MapIcon(icon_def, location);
  }
  
  return (MapObject *)icon;
}


/*********************************************************************
 * _createPolygonObject() - Create a polygon map object from a polygon
 *                          or arc shape object.
 */

MapObject *Shape2Map::_createPolygonObject(const SHPObject &shape) const
{
  MapPolyline *polyline = new MapPolyline();
  
  for (int i = 0; i < shape.nVertices; ++i)
  {
    MapPoint vertex(shape.padfY[i], shape.padfX[i]);
    
    polyline->addVertex(vertex);
    
  } /* endfor - i */
  
  return (MapObject *)polyline;
}
