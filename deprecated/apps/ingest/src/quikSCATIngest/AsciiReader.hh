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

/************************************************************************
 * AsciiReader: Base class for classes that read observation information from
 *              ASCII files.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef AsciiReader_H
#define AsciiReader_H

#include <cstdio>

#include <map>

#include <toolsa/Path.hh>
#include "quikSCATObs.hh"

using namespace std;


class AsciiReader
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  AsciiReader(const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~AsciiReader();


  /*********************************************************************
   * openFile() - Open the input file.
   *
   * Returns true on success, false on failure.
   */

  bool openFile(const string &ascii_filepath);


  /*********************************************************************
   * endOfFile() - Check if we've reached the end of file.
   *
   * Returns true if end of file, false if not.
   */

  bool endOfFile();

  /*********************************************************************
   * getNextObs() - Read and return a pointer to the next observation from the input file.
   */

  quikSCATObs *getNextObs();

  /*********************************************************************
   * closeFile() - Close the output file.
   */

  virtual inline void closeFile()
  {
    if (_asciiFile != 0)
      fclose(_asciiFile);
    _asciiFile = 0;
  }


protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const double MISSING_DATA_VALUE;
  static const int INPUT_LINE_LEN;
  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  static const int MAX_INPUT_LINE_LEN;

  typedef enum
  {
    OBS_TOKEN_TYPE,
    LAT_TOKEN_NUM,
    LON_TOKEN_NUM,
    WIND_DIR_TOKEN_NUM,
    WIND_SPEED_TOKEN_NUM,
    RAIN_FLAG_TOKEN_NUM,
    NSOL_FLAG_TOKEN_NUM,
    OBS_DATE_TOKEN_NUM,
    OBS_TIME_TOKEN_NUM,
    OBS_SENSOR_TOKEN_NUMBER,
    NUM_TOKENS
  } input_tokens_t;

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  FILE *_asciiFile;
  string _asciiFilePath;
  
  char *_nextInputLine;

  ///////////////////////
  // Protected methods //
  ///////////////////////


  /*********************************************************************
   * _parseObsTime() - Set the observation time value based on the given
   *                   token values.
   */

  DateTime _parseObsTime(const string obs_date_token,
			 const string obs_time_token);

  /*********************************************************************
   * _parseLat() - Set the latitude value based on the given token values.
   */

  double _parseLat(const string deg_token);
  
  /*********************************************************************
   * _parseLon() - Set the longitude value based on the given token values.
   */

  double _parseLon(const string deg_token);

    /*********************************************************************
   * _parseWindSpeed() - Set the wind speed value based on the given token
   *                     value.
   */

  double _parseWindSpeed(const string wind_speed_token);
  
  /*********************************************************************
   * _parseWindDir() - Set the wind direction value based on the given
   *                   token value.
   */

  double _parseWindDir(const string wind_dir_token);

  /*********************************************************************
   * _parseRainFlag() - Set the rain flag value based on the given
   *                    token value.
   */

  bool _parseRainFlag(const string rain_flag_token);

  /*********************************************************************
   * _parseNsolFlag() - Set the nsol flag value based on the given
   *                    token value.
   */

  bool _parseNsolFlag(const string nsol_flag_token);

};

#endif
