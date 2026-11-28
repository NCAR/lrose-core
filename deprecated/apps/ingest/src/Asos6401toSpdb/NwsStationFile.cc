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
//   $Id: NwsStationFile.cc,v 1.4 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NwsStationFile: Class for classes that control National Weather
 *                 Service-format files with station location information.
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

#include "NwsStationFile.hh"

using namespace std;


// Globals

const int NwsStationFile::MAX_TOKENS = 20;
const int NwsStationFile::MAX_TOKEN_LEN = 80;


/*********************************************************************
 * Constructors
 */

NwsStationFile::NwsStationFile(const bool debug_flag) :
  StationFile(debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

NwsStationFile::~NwsStationFile()
{
}


/*********************************************************************
 * init() - Initialize the object.
 *
 * Returns true on success, false on failure.
 */

bool NwsStationFile::init(const alt_units_t alt_units)
{
  _altitudeUnits = alt_units;

  return true;
}



/*********************************************************************
 * readFile() - Read the stations from the given file.
 *
 * Returns true on success, false on failure.
 */

bool NwsStationFile::readFile(const string &station_file_path)
{
  static const string method_name = "NwsStationFile::readFile()";
  
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
    cerr << "---> station line: <" << line << ">" << endl;
    
    // Increment the line number

    ++ line_num;
    
    // Don't process comment lines
    
    if (line[0] == '#')
      continue;

    // Parse the line
    
    if (STRparse_delim(line, tokens, BUFSIZ, ";",
		       MAX_TOKENS, MAX_TOKEN_LEN) != 14)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error parsing line " << line_num << " of station file" << endl;
      cerr << "Line: " << line << endl;
      cerr << "Skipping station" << endl;
      
      continue;
    }
    
    // Add the station to the station list

    station_info_t station_info;
    
    station_info.station_id = tokens[0];
    station_info.lat = _degMinSec2Deg(tokens[7]);
    station_info.lon = _degMinSec2Deg(tokens[8]);
    if (strlen(tokens[10]) == 0)
      station_info.alt = STATION_NAN;
    else
      station_info.alt = atof(tokens[10]);
    
    if (station_info.lat == STATION_NAN ||
	station_info.lon == STATION_NAN)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error interpreting lat/lon location of station" << endl;
      cerr << "Line: " << line << endl;
      cerr << "Skipping station" << endl;
      
      continue;
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

/*********************************************************************
 * _degMinSec2Deg() - Convert the given lat/lon string from deg-min-sec
 *                    to degrees.
 *
 * Returns the converted value.
 */

double NwsStationFile::_degMinSec2Deg(const string &deg_min_sec_str)
{
  static const string method_name = "NwsStationFile::_degMinSec2Deg()";
  
  int deg;
  int min;
  int sec;
  char hemisphere[BUFSIZ];
  
  double degrees;
  
  if (sscanf(deg_min_sec_str.c_str(), "%d-%d-%d%s",
	     &deg, &min, &sec, hemisphere) == 4)
    degrees = (double)deg + ((double)min / 60.0) + ((double)sec / 3600.0);
  else if (sscanf(deg_min_sec_str.c_str(), "%d-%d%s",
		  &deg, &min, hemisphere) == 3)
    degrees = (double)deg + ((double)min / 60.0);
  else
    return STATION_NAN;
  
  if (hemisphere[0] == 'S' || hemisphere[0] == 'W')
    degrees *= -1.0;
  else if (hemisphere[0] != 'N' && hemisphere[0] != 'E')
  {
    cerr << "*** Invalid hemisphere value: " << hemisphere << endl;
    return STATION_NAN;
  }
  
  return degrees;
}
