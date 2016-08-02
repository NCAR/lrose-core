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
//   $Date: 2016/03/07 18:28:25 $
//   $Id: ShapeAscii2Map.cc,v 1.4 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ShapeAscii2Map : ShapeAscii2Map program class
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
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
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "ShapeAscii2Map.hh"

#include "LocationProcessor.hh"
#include "PolylineProcessor.hh"
#include "SimpleLabelProcessor.hh"

using namespace std;


// Global variables

ShapeAscii2Map *ShapeAscii2Map::_instance = (ShapeAscii2Map *)NULL;


/*********************************************************************
 * Constructor
 */

ShapeAscii2Map::ShapeAscii2Map(int argc, char **argv)
{
  const string method_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ShapeAscii2Map *)NULL);
  
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

ShapeAscii2Map::~ShapeAscii2Map()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

ShapeAscii2Map *ShapeAscii2Map::Inst(int argc, char **argv)
{
  if (_instance == (ShapeAscii2Map *)NULL)
    new ShapeAscii2Map(argc, argv);
  
  return(_instance);
}

ShapeAscii2Map *ShapeAscii2Map::Inst()
{
  assert(_instance != (ShapeAscii2Map *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool ShapeAscii2Map::init()
{
  // Initialize the input processor

  switch (_params->input_type)
  {
  case Params::INPUT_POLYLINES :
  {
    _inputProcessor = new PolylineProcessor(_params->header_lines,
					    _params->debug);
    break;
  }
  
  case Params::INPUT_LOCATIONS :
  {
    vector< MapPointOffset > icon_pts;
    
    for (int i = 0; i < _params->icon_points_n; ++i)
    {
      int x = _params->_icon_points[i].x;
      int y = _params->_icon_points[i].y;
      
      MapPointOffset point;
      
      if (x != 9999 || y != 9999)
      {
	point.x_offset = x;
	point.y_offset = y;
      }
      
      icon_pts.push_back(point);
    }
    
    MapIconDef *icon_def = new MapIconDef(_params->icon_name,
					  icon_pts);
    
    _inputProcessor = new LocationProcessor(icon_def,
					    _params->header_lines,
					    _params->debug);
    break;
  }
  
  case Params::INPUT_SIMPLE_LABELS :
  {
    _inputProcessor = new SimpleLabelProcessor(_params->header_lines,
					       _params->debug);
  }
  
  } /* endswitch - _params->input_type */
  
  return true;
}


/*********************************************************************
 * run()
 */

void ShapeAscii2Map::run()
{
  static const string method_name = "ShapeAscii2Map::run()";

  Map *map;

  if ((map = _inputProcessor->readMap(_args->getInputFileName())) == 0)
    return;

  map->write(stdout);
  
  delete map;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

