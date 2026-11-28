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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:22:59 $
//   $Id: AncStationFile.cc,v 1.2 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AncStationFile: Class for classes that control AutoNowcast-format
 *                 files with station location information.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>

#include "AncStationFile.hh"

using namespace std;


// Globals

const int AncStationFile::MAX_TOKENS = 10;
const int AncStationFile::MAX_TOKEN_LEN = 80;


/*********************************************************************
 * Constructors
 */

AncStationFile::AncStationFile(const bool debug_flag) :
  StationFile(debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

AncStationFile::~AncStationFile()
{
}


/*********************************************************************
 * init() - Initialize the object.
 *
 * Returns true on success, false on failure.
 */

bool AncStationFile::init(const alt_units_t alt_units)
{
  _altitudeUnits = alt_units;

  return true;
}



/*********************************************************************
 * readFile() - Read the stations from the given file.
 *
 * Returns true on success, false on failure.
 */

bool AncStationFile::readFile(const string &station_file_path)
{
  static const string method_name = "AncStationFile::readFile()";
  
  // Open the station file

  FILE *station_file;
  
  if ((station_file = fopen(station_file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot open station location file: "
	 << station_file_path << endl;

    return false;
  }
   
  // Allocate space for the tokens used for parsing the file lines

  char **tokens = new char*[MAX_TOKENS];
 
  for (int i = 0; i < MAX_TOKENS; ++i)
    tokens[i] = new char[MAX_TOKEN_LEN];
  
  // Read the station information

  char line[BUFSIZ];
  int line_num = 0;
  
  while (fgets(line, BUFSIZ, station_file) != 0)
  {
    // Increment the line number

    ++ line_num;
    
    // Don't process comment lines
    
    if (line[0] == '#')
      continue;

    // Parse the line
    
    if (STRparse_delim(line, tokens, BUFSIZ, ",",
		       MAX_TOKENS, MAX_TOKEN_LEN) != 4)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error parsing line " << line_num << " of station file" << endl;
      cerr << "Line: " << line << endl;
      
      continue;
    }
    
    // Add the station to the station list

    station_info_t station_info;
    
    station_info.station_id = tokens[0];
    station_info.lat = atof(tokens[1]);
    station_info.lon = atof(tokens[2]);
    station_info.alt = atof(tokens[3]);
    
    if (station_info.alt == -999.0 || station_info.alt == 999.0)
    {
      station_info.alt = STATION_NAN;
    }
    else
    {
      switch (_altitudeUnits)
      {
      case ALT_UNITS_FEET :
	station_info.alt *= M_PER_FT;
	break;
      case ALT_UNITS_METERS :
	break;
      }
    }
    
    _stationList[station_info.station_id] = station_info;
    
  } /* endwhile - fgets */

  // Free the space for the parsing tokens

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] tokens[i];
  
  delete [] tokens;
  
  // Close the station file

  fclose(station_file);

  // Check to see if we got any stations

  if (_stationList.size() <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No suitable locations in file: " << station_file_path << endl;

    return false;
  }

  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
