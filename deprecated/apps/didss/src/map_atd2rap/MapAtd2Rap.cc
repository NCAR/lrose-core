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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: MapAtd2Rap.cc,v 1.5 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MapAtd2Rap.cc: stratiform_filter program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>
#include <cstdio>
#include <cstdlib>

#include <toolsa/os_config.h>
#include <rapformats/Map.hh>
#include <rapformats/MapPoint.hh>
#include <rapformats/MapPolyline.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MapAtd2Rap.hh"
using namespace std;

// Global variables

MapAtd2Rap *MapAtd2Rap::_instance = (MapAtd2Rap *)NULL;

// Global constants

const int FOREVER = true;


/*********************************************************************
 * Constructor
 */

MapAtd2Rap::MapAtd2Rap(int argc, char **argv)
{
  // Make sure the singleton wasn't already created.

  assert(_instance == (MapAtd2Rap *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
}


/*********************************************************************
 * Destructor
 */

MapAtd2Rap::~MapAtd2Rap()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MapAtd2Rap *MapAtd2Rap::Inst(int argc, char **argv)
{
  if (_instance == (MapAtd2Rap *)NULL)
    new MapAtd2Rap(argc, argv);
  
  return(_instance);
}

MapAtd2Rap *MapAtd2Rap::Inst()
{
  assert(_instance != (MapAtd2Rap *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void MapAtd2Rap::run()
{
  fprintf(stderr, "Processing file <%s> into file <%s>\n",
	  _args->inputFilename.c_str(), _args->outputFilename.c_str());
  
  // Read the ATD map

  Map *map = _readAtdMap(_args->inputFilename.c_str());
  
  // Write the map to the output file

  FILE *rap_file;
  
  if ((rap_file = fopen(_args->outputFilename.c_str(), "w"))
      == (FILE *)NULL)
  {
    fprintf(stderr, "ERROR opening RAP output file\n");
    perror(_args->outputFilename.c_str());
    
    exit(-1);
  }
  
  map->write(rap_file);
  
  fclose(rap_file);
  
  delete map;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _readAtdMap() - Read the ATD map from the indicated file.
 */

Map *MapAtd2Rap::_readAtdMap(string atd_filename)
{
  // Open the input file

  FILE *atd_file;
  if ((atd_file = fopen(atd_filename.c_str(), "r"))
      == (FILE *)NULL)
  {
    fprintf(stderr, "ERROR opening ATD input file\n");
    perror(atd_filename.c_str());
    
    exit(-1);
  }
  
  // Create the Map object to return

  Map *map = new Map();
  
  // Process each polyline in the ATD map file

  char input_line[BUFSIZ];
  
  int line_num = 0;
  
  while (fgets(input_line, BUFSIZ, atd_file) != (char *)NULL)
  {
    line_num++;
    
    int num_vals;
    float max_lat, min_lat;
    float max_lon, min_lon;
    
    int num_tokens = sscanf(input_line, "%d %f %f %f %f",
			    &num_vals,
			    &max_lat, &min_lat,
			    &max_lon, &min_lon);
    
    // Check for blank lines

    if (num_tokens <= 0)
      continue;
    
    // Check for errors on the line

    if (num_tokens != 5)
    {
      fprintf(stderr, 
	      "ERROR scanning line %d of input file: <%s>\n",
	      line_num, input_line);
      exit(-1);
    }
    
    // Create the polyline object

    MapPolyline *polyline = new MapPolyline();
    
    // Process each vertex of the polyline

    int num_vals_read = 0;
    
    while (num_vals_read < num_vals)
    {
      if (fgets(input_line, BUFSIZ, atd_file) == (char *)NULL)
      {
	fprintf(stderr,
		"ERROR reading vertex line of input file\n");
	perror(_args->inputFilename.c_str());
	
	exit(-1);
      }
      
      line_num++;
      
      char *input_line_ptr = input_line;
      char *end_ptr;
      double value_read;
      bool read_lat = true;
      
      while (true)
      {
	value_read = strtod(input_line_ptr, &end_ptr);
	
	// Check for the end of the input line

	if (value_read == 0.0 && end_ptr == input_line_ptr)
	  break;
	
	// Process the read value

	double lat, lon;
	
	if (read_lat)
	{
	  lat = value_read;

	  read_lat = false;
	}
	else
	{
	  lon = value_read;

	  MapPoint map_point(lat, lon);
	  polyline->addVertex(map_point);
	  
	  read_lat = true;
	}
	
	// Increment the appropriate pointers, etc.

	num_vals_read++;
	input_line_ptr = end_ptr;
	
      }
      
    }  /* endwhile -- reading each vertex in the polyline */
    
    map->addObject(polyline);
    
  }  /* endwhile -- reading each polyline in the file */
  
  fclose(atd_file);
  
  return map;
  
}
