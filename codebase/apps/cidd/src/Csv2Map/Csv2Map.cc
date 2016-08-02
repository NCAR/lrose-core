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
//   $Id: Csv2Map.cc,v 1.2 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Csv2Map : Csv2Map program class
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2014
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <cstdio>
#include <vector>

#include <rapformats/Map.hh>
#include <rapformats/MapObject.hh>
#include <rapformats/MapPoint.hh>
#include <rapformats/MapPointOffset.hh>
#include <rapformats/MapIcon.hh>
#include <rapformats/MapIconPoint.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "Csv2Map.hh"

using namespace std;


// Global variables

Csv2Map *Csv2Map::_instance = (Csv2Map *)NULL;

const int Csv2Map::BUFFER_SIZE = 1024;

/*********************************************************************
 * Constructor
 */

Csv2Map::Csv2Map(int argc, char **argv) :
  _maxTokens(0),
  _createLabelsMap(false)
{
  const string method_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Csv2Map *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

Csv2Map::~Csv2Map()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);

  // Free the space used for the tokens

  for (int i = 0; i < _maxTokens; ++i)
    delete [] _tokens[i];
  
  delete [] _tokens;
  
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Csv2Map *Csv2Map::Inst(int argc, char **argv)
{
  if (_instance == (Csv2Map *)NULL)
    new Csv2Map(argc, argv);
  
  return(_instance);
}

Csv2Map *Csv2Map::Inst()
{
  assert(_instance != (Csv2Map *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool Csv2Map::init()
{
  // Set the maximum tokens value.  This value depends on tne token
  // indices specified in the parameter file.  First set this to the
  // index of the last needed token on each line, then add 1 to get the
  // number of tokens from the token location.

  _maxTokens = _params->lat_index;
  if (_maxTokens < _params->lon_index)
    _maxTokens = _params->lon_index;
  if (_maxTokens < _params->label_index)
    _maxTokens = _params->label_index;
  if (_maxTokens < _params->type_index)
    _maxTokens = _params->type_index;
  ++_maxTokens;
  
  // Now allocate the space for the tokens

  _tokens = new char*[_maxTokens];
  
  for (int i = 0; i < _maxTokens; ++i)
    _tokens[i] = new char[BUFFER_SIZE];
  
  // Initialize the icon definition

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
    
  _iconDef = new MapIconDef(_params->icon_name, icon_pts);
    
  // Initialize the null icon definition

  vector< MapPointOffset > null_icon_pts;
  MapPointOffset null_point;
  null_icon_pts.push_back(null_point);
  
  _nullIconDef = new MapIconDef("null", null_icon_pts);
    
  // Set the flag indicating whether we create the labels map

  if (strlen(_params->output_labels_path) != 0 &&
      _params->label_index >= 0)
    _createLabelsMap = true;
  
  return true;
}


/*********************************************************************
 * run()
 */

void Csv2Map::run()
{
  static const string method_name = "Csv2Map::run()";

  // Open the input file

  FILE *input_file;
  if ((input_file = fopen(_params->input_path, "r")) == 0)
  {
    cerr << "ERROR: "<< method_name << endl;
    cerr << "Error opening input file: " << _params->input_path << endl;
    
    return;
  }
  
  // Initialize the maps

  Map icons_map;
  Map labels_map;
  
  icons_map.addObject(_iconDef);
  labels_map.addObject(_nullIconDef);
  
  // Process the lines in the input file

  char line_buffer[BUFFER_SIZE];
  
  while (fgets(line_buffer, BUFFER_SIZE, input_file) != 0)
  {
    // Skip comment lines

    if (line_buffer[0] == '#')
      continue;
    
    // Skip blank lines (or lines containing just a new line)

    if (strlen(line_buffer) < 2)
      continue;
    
    // Remove the last character from the input line.  This should
    // be the newline character, unless our input buffer is too small

    line_buffer[strlen(line_buffer)-1] = '\0';
    
    // Process the line.  We don't really care whether we succeeded or
    // failed since we'll just try to process the next line anyway.

    _processLine(line_buffer, icons_map, labels_map);
  }
  
  // Close the input file

  fclose(input_file);

  // Write the maps to the indicated output files

  _writeMap(icons_map, _params->output_icons_path);
  
  if (_createLabelsMap)
    _writeMap(labels_map, _params->output_labels_path);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processLine()
 */

bool Csv2Map::_processLine(const char *line_buffer,
			   Map &icons_map, Map &labels_map)
{
  static const string method_name = "Csv2Map::_processLine()";
  
  // Parse the input line into tokens

  int tokens_found;
  
  if ((tokens_found = STRparse_delim(line_buffer, _tokens,
				     strlen(line_buffer), ",",
				     _maxTokens, BUFFER_SIZE)) != _maxTokens)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing input line: " << line_buffer << endl;
    cerr << "Expected " << _maxTokens << " tokens, found " << tokens_found << " tokens." << endl;
    
    return false;
  }
  
  // If we are only processing a certain type of line, check for that

  if (_params->type_index >= 0)
  {
    if ((string)_tokens[_params->type_index] !=
	(string)_params->desired_type_string)
      return true;
  }
  
  // Pull the location information from the line

  MapPoint location(atof(_tokens[_params->lat_index]),
		    atof(_tokens[_params->lon_index]));
  
  // Add the icon to the icons map

  MapIcon *icon;
  if (_params->include_labels_in_icons_map &&
      _params->label_index >= 0)
    icon = new MapIcon(_iconDef, location,
		       _tokens[_params->label_index],
		       _params->label_pixel_offset.x_offset,
		       _params->label_pixel_offset.y_offset);
  else
    icon = new MapIcon(_iconDef, location);

  icons_map.addObject(icon);
  
  // Add the label to the labels map

  if (_createLabelsMap)
  {
    MapIcon *label = new MapIcon(_nullIconDef, location,
				 _tokens[_params->label_index],
				 _params->label_pixel_offset.x_offset,
				 _params->label_pixel_offset.y_offset);
    labels_map.addObject(label);
  }
  
  return true;
}


/*********************************************************************
 * _writeMap()
 */

bool Csv2Map::_writeMap(const Map &map, const string &file_path)
{
  static const string method_name = "Csv2Map::_writeMap()";
  
  // Open the output file

  FILE *output_file;
  
  if ((output_file = fopen(file_path.c_str(), "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening map output file: " << file_path << endl;
    
    return false;
  }
  
  // Write the map to the file

  map.write(output_file);
  
  // Close the output file

  fclose(output_file);
  
  return true;
}
