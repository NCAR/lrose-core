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
//   $Date: 2016/03/07 01:23:08 $
//   $Id: ihop3x3Goes2Mdv.cc,v 1.8 2016/03/07 01:23:08 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ihop3x3Goes2Mdv: ihop3x3Goes2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak/Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <cassert>

#include <toolsa/os_config.h>
#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "GridHandler.hh"
#include "ihop3x3Goes2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

ihop3x3Goes2Mdv *ihop3x3Goes2Mdv::_instance = (ihop3x3Goes2Mdv *)NULL;

const int ihop3x3Goes2Mdv::TOKEN_DATE_BEGIN_YY = 1;
const int ihop3x3Goes2Mdv::TOKEN_DATE_LEN_YY = 4;

const int ihop3x3Goes2Mdv::TOKEN_DATE_BEGIN_JULIAN_DAY = 5;
const int ihop3x3Goes2Mdv::TOKEN_DATE_LEN_JULIAN_DAY = 3;

const int ihop3x3Goes2Mdv::TOKEN_DATE_BEGIN_HOUR = 11;
const int ihop3x3Goes2Mdv::TOKEN_DATE_LEN_HOUR = 2;

const int ihop3x3Goes2Mdv::TOKEN_DATE_BEGIN_MIN = 13;
const int ihop3x3Goes2Mdv::TOKEN_DATE_LEN_MIN = 2;

const int ihop3x3Goes2Mdv::TOKEN_DATE_BEGIN_SEC = 15;
const int ihop3x3Goes2Mdv::TOKEN_DATE_LEN_SEC = 2;

const int ihop3x3Goes2Mdv::TOKEN_LAT_BEGIN = 19;
const int ihop3x3Goes2Mdv::TOKEN_LAT_LEN = 7;

const int ihop3x3Goes2Mdv::TOKEN_LON_BEGIN = 27;
const int ihop3x3Goes2Mdv::TOKEN_LON_LEN = 7;

const int ihop3x3Goes2Mdv::TOKEN_TEMP_BEGIN = 38;
const int ihop3x3Goes2Mdv::TOKEN_TEMP_LEN = 6;

const int ihop3x3Goes2Mdv::TOKEN_DEWPOINT_BEGIN = 47;
const int ihop3x3Goes2Mdv::TOKEN_DEWPOINT_LEN = 6;

const int ihop3x3Goes2Mdv::TOKEN_PRESSURE_BEGIN = 56;
const int ihop3x3Goes2Mdv::TOKEN_PRESSURE_LEN = 6;

const int ihop3x3Goes2Mdv::TOKEN_HEIGHT_BEGIN = 64;
const int ihop3x3Goes2Mdv::TOKEN_HEIGHT_LEN = 5;

const int ihop3x3Goes2Mdv::TOKEN_RETRIEVAL_TYPE_BEGIN = 72;
const int ihop3x3Goes2Mdv::TOKEN_RETRIEVAL_TYPE_LEN = 2;

const int ihop3x3Goes2Mdv::MINIMUM_INPUT_LENGTH = 74;


/*********************************************************************
 * Constructor
 */

ihop3x3Goes2Mdv::ihop3x3Goes2Mdv(int argc, char **argv)
{
  static const string method_name = "ihop3x3Goes2Mdv::ihop3x3Goes2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ihop3x3Goes2Mdv *)NULL);
  
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
  char *params_path = (char *) "unknown";
  
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

ihop3x3Goes2Mdv::~ihop3x3Goes2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

ihop3x3Goes2Mdv *ihop3x3Goes2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (ihop3x3Goes2Mdv *)NULL)
    new ihop3x3Goes2Mdv(argc, argv);
  
  return(_instance);
}

ihop3x3Goes2Mdv *ihop3x3Goes2Mdv::Inst()
{
  assert(_instance != (ihop3x3Goes2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ihop3x3Goes2Mdv::init()
{
  static const string method_name = "ihop3x3Goes2Mdv::init()";
  
  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ihop3x3Goes2Mdv::run()
{
  static const string method_name = "ihop3x3Goes2Mdv::run()";
  
  if (_args->isRealtime())
  {
    cerr << "REALTIME mode not yet implemented..." << endl;
  }
  else
  {
    // Process the files specified on the command line

    while (true)
    {
      const string file_name = _args->nextFile();
      
      if (file_name == "")
	break;
      
      _processData(file_name);
    } /* endwhile - still files to process */
    
  }
  
}


/**********************************************************************
 *              Private Methods
 **********************************************************************/

/*********************************************************************
 * _convertDouble()
 * 
 * Pulls indicated token out of input line, converts it to a double, and
 * returns true upon success
 */

bool ihop3x3Goes2Mdv::_convertDouble(const char* inputLine, const int begin, const int numChars, double &return_value) const
{
  static char token[100];
  char *endptr;


  STRcopy(token, &(inputLine[begin]), numChars + 1);

  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "token: <" << token << ">" << endl;

  //test to see if the token consists of blank spaces; if so, set to the
  //missing data value

  bool allBlanks = true;

  for (int i=0; i < numChars; ++i)
    {
      if (token[i] != ' ')
	{
	  allBlanks = false;
	  break;
	}
    }

  if (allBlanks)
    {
      return_value = GridHandler::MISSING_DATA_VALUE;
      return true;
    }

  return_value = strtod(token, &endptr);

  //if these values are the same, it means the whole token had errors in the conversion
  if (endptr == token)
    {
      if (_params->debug >= Params::DEBUG_NORM)
	{
	  cerr << "endptr and token are the same: <" << endptr << ">." << endl;
	  cerr << "Setting to MISSING_DATA_VALUE." << endl;
	}
      return_value = GridHandler::MISSING_DATA_VALUE;
      return true;
    }

  if (endptr[0] != '\0')
    {
      if (_params->debug >= Params::DEBUG_NORM)
	{
	  cerr << "Error parsing token." << endl;
	  cerr << "endptr: <" << endptr << ">" << endl;
	  cerr << "token: <" << token << ">" << endl;
	}
      return false;
    }
  return true;
}

/*********************************************************************
 * _convertDate()
 * 
 * Pulls date information out of input line, calls convertInt() for each token,
 * returns a DateTime object
 */

DateTime ihop3x3Goes2Mdv::_convertDate(char* inputLine)
{
  
  int year, day, hour, min, sec;
  
  //some of the time fields have blanks instead of 0's at the beginning of the field

  for (int i = TOKEN_DATE_BEGIN_YY; i <= TOKEN_DATE_BEGIN_YY + TOKEN_DATE_LEN_YY; ++i)
    if (inputLine[i] == ' ')
      inputLine[i] = '0';

  for (int i = TOKEN_DATE_BEGIN_JULIAN_DAY; i <= TOKEN_DATE_BEGIN_JULIAN_DAY + TOKEN_DATE_LEN_JULIAN_DAY; ++i)
    if (inputLine[i] == ' ')
      inputLine[i] = '0';

  for (int i = TOKEN_DATE_BEGIN_HOUR; i <= TOKEN_DATE_BEGIN_HOUR + TOKEN_DATE_LEN_HOUR; ++i)
    if (inputLine[i] == ' ')
      inputLine[i] = '0';

  for (int i = TOKEN_DATE_BEGIN_MIN; i <= TOKEN_DATE_BEGIN_MIN + TOKEN_DATE_LEN_MIN; ++i)
    if (inputLine[i] == ' ')
      inputLine[i] = '0';

  for (int i = TOKEN_DATE_BEGIN_SEC; i <= TOKEN_DATE_BEGIN_SEC + TOKEN_DATE_LEN_SEC; ++i)
    if (inputLine[i] == ' ')
      inputLine[i] = '0';

  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_YY, TOKEN_DATE_LEN_YY, year))
    {
      cerr << "Error parsing year token." << endl;
      return DateTime::NEVER;
    }

  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_JULIAN_DAY, TOKEN_DATE_LEN_JULIAN_DAY, day))
    {
      cerr << "Error parsing day token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_HOUR, TOKEN_DATE_LEN_HOUR, hour))
    {
      cerr << "Error parsing hour token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_MIN, TOKEN_DATE_LEN_MIN, min))
    {
      cerr << "Error parsing min token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_SEC, TOKEN_DATE_LEN_SEC, sec))
    {
      cerr << "Error parsing sec token." << endl;
      return DateTime::NEVER;
    }
  DateTime soundingDate;

  return soundingDate.setByDayOfYear(year, day, hour, min, sec);
}

/*********************************************************************
 * _convertInt()
 * 
 * Pulls indicated token out of input line, converts it to an int, and
 * returns true upon success
 */

bool ihop3x3Goes2Mdv::_convertInt(const char* inputLine, const int begin, const int numChars, int &return_value)
{
  static char token[100];
  char *endptr;

  STRcopy(token, &(inputLine[begin]), numChars + 1);

  return_value = strtol(token, &endptr, 10);

  if (endptr[0] != '\0')
    {
      cerr << "Error parsing date token." << endl;
      return false;
    }
  return true;
}

/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool ihop3x3Goes2Mdv::_processData(const string& file_name)
{
  static const string method_name = "ihop3x3Goes2Mdv::_processData()";
  
  static const int INPUT_LINE_LEN = 1024;

  char input_line[INPUT_LINE_LEN];
  
  GridHandler gridHandler;
  MdvxPjg projection;
  
  switch(_params->xy_grid.proj_type)
    {
    case Params::PROJ_FLAT:
      projection.initFlat(_params->xy_grid.origin_lat, 
			  _params->xy_grid.origin_lon,
			  _params->xy_grid.rotation,
			  _params->xy_grid.nx,
			  _params->xy_grid.ny,
			  _params->pressure_levels_n,
			  _params->xy_grid.dx,
			  _params->xy_grid.dy,
			  1.0,
			  _params->xy_grid.minx,
			  _params->xy_grid.miny,
			  0.0);
      break;
    case Params::PROJ_LATLON:
      projection.initLatlon(_params->xy_grid.nx,
			    _params->xy_grid.ny,
			    _params->pressure_levels_n,
			    _params->xy_grid.dx,
			    _params->xy_grid.dy,
			    1.0,
			    _params->xy_grid.minx,
			    _params->xy_grid.miny,
			    0.0);
      break;
    }
  
  vector<double> pressureLevels;

  for(int i = 0; i < _params->pressure_levels_n; ++i)
    {
      pressureLevels.push_back(_params->_pressure_levels[i]);
    }

  gridHandler.init(projection, pressureLevels);
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "*** Processing file <" << file_name << ">" << endl;

  // Open the input file

  FILE *input_file;

  if ((input_file = ta_fopen_uncompress((char *)file_name.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << file_name << endl;
    
    return false;
  }
  
  // Skip any header lines

  for (int line_num = 0; line_num < _params->num_header_lines; ++line_num)
    {
      fgets(input_line, INPUT_LINE_LEN, input_file);
      if (_params->debug >= Params::DEBUG_VERBOSE)
	cerr << "Skipping header line: " << input_line << endl;
    }

  // Process the data lines

  int numSoundings = 0;
  int numSuccessfulSoundings = 0;

  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
  {
    ++numSoundings;

    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Processing line: " << input_line << endl;

    // Check to see if the line meets the minimum input length requirement

    if (strlen(input_line) < (size_t)MINIMUM_INPUT_LENGTH)
      {
	if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "Skipping line; too short to process." << endl;
	continue;
      }

    // Create the structure with the file information; parse the tokens in the line
    
    GridHandler::sounder_data_t sounder_data;
    
    //time is in UTC

    DateTime report_time = _convertDate(input_line);
    if (report_time == DateTime::NEVER)
      {
	cerr << "Error with report time. Skipping line." << endl;
	continue;
      }
    
    sounder_data.date_time = report_time;

    if (!_convertDouble(input_line, TOKEN_LAT_BEGIN, TOKEN_LAT_LEN, sounder_data.latitude))
      {
	cerr << "Error converting latitude token to double." << endl;
	continue;
      }

    if (!_convertDouble(input_line, TOKEN_LON_BEGIN, TOKEN_LON_LEN, sounder_data.longitude))
      {
	cerr << "Error converting longitude token to double." << endl;
	continue;
      }

    // set longitude to negative value since we don't use positive west
    // we use negative west

    sounder_data.longitude = sounder_data.longitude * -1.0;

    if (!_convertDouble(input_line, TOKEN_TEMP_BEGIN, TOKEN_TEMP_LEN, sounder_data.temp))
      {
	cerr << "Error converting temperature token to double." << endl;
	continue;
      }

    if (!_convertDouble(input_line, TOKEN_DEWPOINT_BEGIN, TOKEN_DEWPOINT_LEN, sounder_data.dewpoint))
      {
	cerr << "Error converting dewpoint token to double." << endl;
	continue;
      }

    if (!_convertDouble(input_line, TOKEN_PRESSURE_BEGIN, TOKEN_PRESSURE_LEN, sounder_data.pressure))
      {
	cerr << "Error converting pressure token to double." << endl;
	continue;
      }

    if (!_convertInt(input_line, TOKEN_HEIGHT_BEGIN, TOKEN_HEIGHT_LEN, sounder_data.height))
      {
	cerr << "Error converting height token to int." << endl;
	continue;
      }

    if (!_convertInt(input_line, TOKEN_RETRIEVAL_TYPE_BEGIN, TOKEN_RETRIEVAL_TYPE_LEN, sounder_data.retrieval_type))
      {
	cerr << "Error converting retrieval token to int." << endl;
	continue;
      }

    if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "time [UTC]: <" << sounder_data.date_time << ">" << endl; 
	cerr << "latitude [DEG]: <" << sounder_data.latitude << ">" << endl;
	cerr << "longitude [DEG]: <" << sounder_data.longitude << ">" << endl;
	cerr << "temp [K]: <" << sounder_data.temp << ">" << endl;
	cerr << "dewpoint [K]: <" << sounder_data.dewpoint << ">" << endl;
	cerr << "pressure [MB]: <" << sounder_data.pressure << ">" << endl;
	cerr << "height [M]: <" << sounder_data.height << ">" << endl;
	cerr << "retrieval_type [21 = clear]: <" << sounder_data.retrieval_type << ">" << endl;
      }

    // update grid

    if (!gridHandler.updateGrid(sounder_data))
      {
	cerr << "Update grid unsuccessful." << endl;
      }

    ++numSuccessfulSoundings;

  }  /* endwhile - lines left in input file */

  if (_params->debug >= Params::DEBUG_NORM)
    {
      cerr << "***   Finished processing file: " << file_name << endl;
      cerr << "      Number of soundings: " << numSoundings << endl;
      cerr << "      Number of successful soundings: " << numSuccessfulSoundings << endl;
    }
  fclose(input_file); 

  if (!gridHandler.writeGrid(_params->output_url, file_name))
    {
      cerr << "Unable to write grid." << endl;
      return false;
    }

  return true;
};
