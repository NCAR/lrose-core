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
//   $Date: 2016/03/07 01:22:59 $
//   $Id: CreateStationList.cc,v 1.4 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CreateStationList: CreateStationList program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CreateStationList.hh"
#include "Params.hh"
using namespace std;


// Global variables

CreateStationList *CreateStationList::_instance =
     (CreateStationList *)NULL;


/*********************************************************************
 * Constructor
 */

CreateStationList::CreateStationList(int argc, char **argv)
{
  static const string method_name = "CreateStationList::CreateStationList()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CreateStationList *)NULL);
  
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
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

CreateStationList::~CreateStationList()
{
  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

CreateStationList *CreateStationList::Inst(int argc, char **argv)
{
  if (_instance == (CreateStationList *)NULL)
    new CreateStationList(argc, argv);
  
  return(_instance);
}

CreateStationList *CreateStationList::Inst()
{
  assert(_instance != (CreateStationList *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CreateStationList::init()
{
  static const string method_name = "CreateStationList::init()";
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CreateStationList::run()
{
  static const string method_name = "CreateStationList::run()";
  
  // Open the input file

  FILE *input_file;

  if ((input_file = fopen(_params->input_file, "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file for read" << endl;
    perror(_params->input_file);
    
    return;
  }
  
  // Open the output file

  FILE *output_file;
  
  if ((output_file = fopen(_params->output_file, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file for write" << endl;
    perror(_params->output_file);
    
    return;
  }
  
  // Process each of the lines in the input file

  static const int INPUT_LINE_LEN = 1024;
  
  char input_line[INPUT_LINE_LEN];
  
  int line_num = 0;
  
  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
  {
    ++line_num;
    
    // Skip comment lines

    if (input_line[0] == '!')
      continue;
    
    
    char icao[5];
    int lat_degrees;
    int lat_minutes;
    char lat_direction;
    int lon_degrees;
    int lon_minutes;
    char lon_direction;
    int elev;
    
    STRcopy(icao, &input_line[20], 5);

    //
    // Skip entries sans 4 character ID.
    //
    if (!(strncmp("    ", icao, 4))){
      continue;
    }

    lat_degrees = atoi(&input_line[39]);
    lat_minutes = atoi(&input_line[42]);
    lat_direction = input_line[44];
    
    lon_degrees = atoi(&input_line[47]);
    lon_minutes = atoi(&input_line[51]);
    lon_direction = input_line[53];
    
    elev = atoi(&input_line[55]);
    
    double lat;
    double lon;
    
    lat = (double)lat_degrees + (double)lat_minutes / 60.0;
    lon = (double)lon_degrees + (double)lon_minutes / 60.0;
    
    switch (lat_direction)
    {
    case 'S' :
    case 's' :
      lat = -lat;
      break;

    case 'N' :
    case 'n' :
      // Do nothing
      break;
      
    default:
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid latitude direction (" << lat_direction <<
	") specified on line " << line_num << endl;
      continue;
    } /* endswitch - lat_direction */
    
    switch (lon_direction)
    {
    case 'W' :
    case 'w' :
      lon = -lon;
      break;

    case 'E' :
    case 'e' :
      // Do nothing
      break;
      
    default:
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid longitude direction (" << lon_direction <<
	") specified on line " << line_num << endl;
      continue;
    } /* endswitch - lon_direction */
    
    // Ignore stations not in the specified grid

    if (lat < _params->grid_limits.min_lat ||
	lat > _params->grid_limits.max_lat ||
	lon < _params->grid_limits.min_lon ||
	lon > _params->grid_limits.max_lon)
      continue;

    // Write the line to the output file

    fprintf(output_file, "%s, %f, %f, %d\n",
	    icao, lat, lon, elev);
    
  } /* endwhile - fgets */
  
  // Close the files

  fclose(input_file);
  fclose(output_file);
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
