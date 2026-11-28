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
/**
 *
 * @file FileHandler.hh
 *
 * @class FileHandler
 *
 * Class for processing the station reports in an SMOS file.
 *  
 * @date 10/5/2009
 *
 */

#ifndef FileHandler_HH
#define FileHandler_HH

#include <vector>

#include <rapformats/station_reports.h>

using namespace std;

/** 
 * @class FileHandler
 */

class FileHandler
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] file_path Path for input file to process.
   * @param[in] input_missing_values List of values used to flag missing
   *                                 data in the input file.
   * @param[in] negate_lon_values Flag indicating whether to negate the
   *                              longitude values found in the input file.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   */

  FileHandler(const string &file_path,
	      const vector< double > input_missing_values,
	      const bool negate_long_values,
	      const bool debug_flag = false,
	      const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~FileHandler(void);
  

  /**
   * @brief Initialize the sounding file.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  

  /**
   * @brief Process the soundings in the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool processFile();
  

  /**
   * @brief Get the station reports read from the file.  processFile() must be
   *        called before using this method.
   *
   * @return Returns the list of station reports read from the file.
   */

  vector< station_report_t > &getReports()
  {
    return _reports;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Maximum number of tokens on an input line.
   */

  static const int MAX_TOKENS;
  

  /**
   * @brief Maximum number of character in a token.
   */

  static const int MAX_TOKEN_LEN;
  

  /**
   * @brief Index values for the tokens on the input line.
   */

  typedef enum
  {
    STATION_ID_TOKEN,
    STATION_LON_TOKEN,
    STATION_LAT_TOKEN,
    STATION_ALT_M_TOKEN,
    SURF_PRESS_MB_TOKEN,
    TEMPERATURE_C_TOKEN,
    DEWPOINT_C_TOKEN,
    PRECIP_ACCUM_MM_TOKEN,
    WIND_SPEED_M_PER_SEC_TOKEN,
    WIND_DIR_DEG_TOKEN,
    NUM_TOKENS
  } tokens_t;
  
    
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  

  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  

  /**
   * @brief List of missing data values in the input file.
   */

  vector< double > _inputMissingValues;
  

  /**
   * @brief The path to the EOL sounding file.
   */

  string _filePath;
  

  /**
   * @brief Input file data time.
   */

  DateTime _fileTime;
  

  /**
   * @brief Pointer to the input file.
   */

  FILE *_inputFile;
  

  /**
   * @brief Flag indicating whether to negate the longitude values found
   *        in the input file.
   */

  bool _negateLonValues;
  

  /**
   * @brief Token buffer for parsing input lines.
   */

  char **_tokens;
  

  /**
   * @brief List of station reports in the input file.
   */

  vector< station_report_t > _reports;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Extract the data time from the file name.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getFileTimeFromPath();
  

  /**
   * @brief Initialize the given station report.
   *
   * @param[out] report Station report to initialize.
   */

  void _initReport(station_report_t &report) const;
  

  /**
   * @brief Check the input value to see if it is valid.
   *
   * @param[in] value The input value to check.
   *
   * @return Returns true if the input value is valid, false otherwise.
   */

  bool isValid(const double value) const
  {
    vector< double >::const_iterator missing_value;
    
    for (missing_value = _inputMissingValues.begin();
	 missing_value != _inputMissingValues.end(); ++missing_value)
    {
      if (value == *missing_value)
	return false;
    }
    
    return true;
  }
  

  /**
   * @brief Process the given input line.
   *
   * @param[in] lineptr Input line.
   * @param[in] linelne Input line length.
   */

  void _processLine(const char *lineptr, const size_t linelen);
  

};


#endif
