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
 * Class for processing the soundings in a file.
 *  
 * @date 9/28/2009
 *
 */

#ifndef FileHandler_HH
#define FileHandler_HH

#include <vector>

#include <rapformats/Sndg.hh>

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
   * @param[in] file_path Path for EOL sounding file to process.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   */

  FileHandler(const string &file_path,
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
   * @brief Get the soundings read from the file.  processFile() must be
   *        called before using this method.
   *
   * @return Returns the list of soundings read from the file.
   */

  vector< Sndg > &getSoundings()
  {
    return _soundings;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Label for launch location value in input file.
   */

  static const string LAUNCH_LOCATION_LABEL;
  

  /**
   * @brief Label for launch time value in input file.
   */

  static const string LAUNCH_TIME_LABEL;
  

  /**
   * @brief Maximum number of tokens on an input line.
   */

  static const int MAX_TOKENS;
  

  /**
   * @brief Meximum number of character in a token.
   */

  static const int MAX_TOKEN_LEN;
  

  /**
   * @brief Missing data value in the EOL input file.
   */

  static const double EOL_MISSING_VALUE;
  

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
   * @brief The path to the EOL sounding file.
   */

  string _filePath;
  

  /**
   * @brief Pointer to the input file.
   */

  FILE *_inputFile;
  

  /**
   * @brief Token buffer for parsing input lines.
   */

  char **_tokens;
  

  /**
   * @brief List of soundings in the input file.
   */

  vector< Sndg > _soundings;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Add the point from the given input file line to the given sounding.
   *
   * @param[in] line Input file line.
   * @param[in,out] sounding Sounding to update.
   */

  void _addPoint(const string &line, Sndg &sounding) const;
  

  /**
   * @brief Extract the launch location from the given input file line.
   *
   * @param[in] line Input file line.
   * @param[in,out] sounding Sounding to update.
   */

  void _extractLaunchLocation(const string &line,
			      Sndg &sounding) const;
  

  /**
   * @brief Extract the launch time from the given input file line.
   *
   * @param[in] line Input file line.
   * @param[in,out] sounding Sounding to update.
   */

  void _extractLaunchTime(const string &line,
			  Sndg &sounding) const;
  

  /**
   * @brief Initialize the given sounding.
   *
   * @param[out] sounding Sounding to initialize.
   */

  void _initSounding(Sndg &sounding) const;
  

  /**
   * @brief Check the given file line for a sounding header value.  If the
   *        line contains a header value, update the value in the given
   *        sounding.
   *
   * @param[in] line Current line from input file.
   * @param[in,out] sounding Sounding to update.
   */
   
  void _updateHeaderValue(const string &line,
			  Sndg &sounding) const;
  

};


#endif
