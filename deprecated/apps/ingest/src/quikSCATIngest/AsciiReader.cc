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
 * AsciiReader: Class for reading observation information from
 *              ASCII files.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <physics/physics.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

#include "AsciiReader.hh"

using namespace std;

const double AsciiReader::MISSING_DATA_VALUE = -999.0;
const int AsciiReader::INPUT_LINE_LEN = 1024;
const int AsciiReader::MAX_TOKENS = 50;
const int AsciiReader::MAX_TOKEN_LEN = 20;
const int AsciiReader::MAX_INPUT_LINE_LEN = 1024;

/*********************************************************************
 * Constructors
 */

AsciiReader::AsciiReader(const bool debug_flag):
  _debug(debug_flag),
  _asciiFile(0)
{
  // Allocate space for the next input line

  _nextInputLine = new char[INPUT_LINE_LEN];
  _nextInputLine[0] = '\0';
  cerr << "_nextInputLine = <" << _nextInputLine << ">" << endl;
}
 
/*********************************************************************
 * Destructor
 */

AsciiReader::~AsciiReader()
{
  if (_asciiFile != 0)
    closeFile();
  delete [] _nextInputLine;
}

/*********************************************************************
 * openFile() - Open the input file.
 *
 * Returns true on success, false on failure.
 */

bool AsciiReader::openFile(const string &ascii_filepath)
{
  static const string method_name = "AsciiReader::openFile()";
  
  // Close the last file if that wasn't already done
  
  if (_asciiFile != 0)
    closeFile();

  // See if this is a 0-length file -- we can't process those

  struct stat file_stat;
  if (stat(ascii_filepath.c_str(), &file_stat) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error stating input file: " << ascii_filepath << endl;

      return false;
    }

  if (file_stat.st_size <= 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Empty file encountered: " << ascii_filepath << endl;

      return false;
    }

  // Now open the new file

  if ((_asciiFile = fopen(ascii_filepath.c_str(), "r")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening input ASCII file: " << ascii_filepath << endl;
      
      return false;
    }
  _asciiFilePath = ascii_filepath;

  return true;
}

/*********************************************************************
 * endOfFile() - Check if we've reached the end of file.
 *
 * Returns true if end of file, false if not.
 */

bool AsciiReader::endOfFile()
{
  static const string method_name = "AsciiReader::endOfFile()";

  if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading data line from ASCII file:" << _asciiFilePath << endl;
      cerr << "Assuming end of file..." << endl;
      return true;
    }

  return false;

}


/*********************************************************************
 * getNextObs() - 
 *
 * Returns a pointer to a quikSCATObs object.
 */

quikSCATObs * AsciiReader::getNextObs()
{
  static const string method_name = "AsciiReader::getNextObs()";

  // Extract the tokens from the input line

  char **tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    tokens[i] = new char[MAX_TOKEN_LEN];

  int num_tokens;

  if (_debug)
    {
      cerr << endl;
      cerr << "Method name: " << method_name << endl;
      cerr << "Parsing tokens...." << endl;
      cerr << "Next input line: " << _nextInputLine << endl;
    }
  num_tokens = STRparse(_nextInputLine, tokens, MAX_INPUT_LINE_LEN,
			MAX_TOKENS, MAX_TOKEN_LEN);

  if (num_tokens != NUM_TOKENS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing input line into tokens" << endl;
    cerr << "Input line: <" << _nextInputLine << ">" << endl;
    cerr << "Num tokens: " << num_tokens << endl;
    
    for (int i = 0; i < MAX_TOKENS; ++i)
      delete [] tokens[i];
    delete [] tokens;
  
    return 0;
  }

  // Extract the values from the tokens and set the quikSCATObs obs object's attributes.
  // If the extraction is unsuccessful, the attributes should retain their default values.

  quikSCATObs * obs = new quikSCATObs;

  obs->setObsTime(_parseObsTime(tokens[OBS_DATE_TOKEN_NUM],
			       tokens[OBS_TIME_TOKEN_NUM]));

  obs->setLat(_parseLat(tokens[LAT_TOKEN_NUM]));
  obs->setLon(_parseLon(tokens[LON_TOKEN_NUM]));

  obs->setWindSpeed(_parseWindSpeed(tokens[WIND_SPEED_TOKEN_NUM]));
  obs->setWindDir(_parseWindDir(tokens[WIND_DIR_TOKEN_NUM]));
  obs->setRainFlag(_parseRainFlag(tokens[RAIN_FLAG_TOKEN_NUM]));
  obs->setNsolFlag(_parseNsolFlag(tokens[NSOL_FLAG_TOKEN_NUM]));

  // Reclaim memory

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] tokens[i];
  
  delete [] tokens;

  return obs;
}

/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/


/*********************************************************************
 * _parseObsTime() - Parse the observation time value based on the given
 *                   token values.
 */

DateTime AsciiReader::_parseObsTime(const string obs_date_token,
				    const string obs_time_token)
{
  static const string method_name = "AsciiReader::_parseObsTime()";
  if (_debug)
    cerr << "obs date and time tokens: " << obs_date_token << " "
         << obs_time_token << endl;
  
  int year, month, day;
  
  if (sscanf(obs_date_token.c_str(), "%4d/%2d/%2d",
             &year, &month, &day) != 3)
  {
    cerr << "ERROR: Error parsing date token: " << obs_date_token << endl;
    return DateTime::NEVER;
  }
  
  int hour, minute, second;
  DateTime time;

  if (sscanf(obs_time_token.c_str(), "%2d:%2d:%2d",
             &hour, &minute, &second) != 3)
  {
    cerr << "ERROR: Error parsing time token: " << obs_time_token << endl;
    return DateTime::NEVER;
  }
  
  return time.set(year, month, day, hour, minute, second);
}


/*********************************************************************
 * _parseLat() - Set the latitude value based on the given token value.
 */

double AsciiReader::_parseLat(const string deg_token)
{
  static const string method_name = "AsciiReader::_parseLat()";
  if (_debug)
    cerr << "lat token: " << deg_token << endl;
  //  _latitude = atof(deg_token.c_str()) + (atof(min_token.c_str()) / 60.0);
  return atof(deg_token.c_str());
}


/*********************************************************************
 * _parseLon() - Set the longitude value based on the given token value.
 */

double AsciiReader::_parseLon(const string deg_token)
{
  static const string method_name = "AsciiReader::_parseLon()";
  if (_debug)
    cerr << "lon token: " << deg_token << endl;
  //  _longitude = atof(deg_token.c_str()) + (atof(min_token.c_str()) / 60.0);
  return atof(deg_token.c_str());
}


/*********************************************************************
 * _parseWindSpeed() - Set the wind speed value based on the given token
 *                     value.
 */

double AsciiReader::_parseWindSpeed(const string wind_speed_token)
{
  static const string method_name = "AsciiReader::_parseWindSpeed()";
  if (_debug)
    cerr << "wind speed token: " << wind_speed_token << endl;
  return atof(wind_speed_token.c_str());
}


/*********************************************************************
 * _parseWindDir() - Set the wind direction value based on the given
 *                   token value.
 */

double AsciiReader::_parseWindDir(const string wind_dir_token)
{
  static const string method_name = "AsciiReader::_parseWindDir()";
  if (_debug)
    cerr << "wind dir token: " << wind_dir_token << endl;
  
  return atof(wind_dir_token.c_str());
}


/*********************************************************************
 * _parseRainFlag() - Set the rain flag value based on the given
 *                    token value.
 */

bool AsciiReader::_parseRainFlag(const string rain_flag_token)
{
  static const string method_name = "AsciiReader::_parseRainFlag()";
  if (_debug)
    cerr << "rain flag token: " << rain_flag_token << endl;
  return atof(rain_flag_token.c_str());
}


/*********************************************************************
 * _parseNsolFlag() - Set the nsol flag value based on the given
 *                    token value.
 */

bool AsciiReader::_parseNsolFlag(const string nsol_flag_token)
{
  static const string method_name = "AsciiReader::_parseNsolFlag()";
  if (_debug)
    cerr << "nsol flag token: " << nsol_flag_token << endl;
  return atof(nsol_flag_token.c_str());
}
