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
 * @file RefractCalib.hh
 * @class RefractCalib
 * RefractCalib program object.
 * @date 1/15/2008
 */

#ifndef RefractCalib_HH
#define RefractCalib_HH

#include "Args.hh"
#include "Params.hh"
#include "Calib.hh"
#include <Refract/RefParms.hh>

#include <string>
#include <vector>
#include <sys/stat.h>

/** 
 * @class RefractCalib
 * @brief the Algorithm class
 */
class RefractCalib
{
  
public:

  // constructor

  RefractCalib(int argc, char **argv);
  
  /**
   * Flag indicating whether the program status is currently okay.
   */

  bool okay;
  
  /**
   * @brief Destructor
   */
  virtual ~RefractCalib();
  
  /**
   * @brief Run the program.
   */

  int run();

 private:

  string _progName; /**< program name */
  char *_paramsPath;
  Args _args;
  Params _params; /**< Parameter file paramters. */
  RefParms _refparms;
  Calib _driver; /**< Data processing object. */
  
  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */
  
  int _init();
  
  /**
   * @brief Create the quality field colorscale used in the original
   *        applications.
   */

  void _createQualityColorscale() const;

  /**
   * @brief Create the strength field colorscale used in the original
   *        applications.
   */

  void _createStrengthColorscale() const;

  std::vector<string>_setupFiles(int numFiles,
				 char ** fileList,
				 Params::Time_t *timeRange,
				 const std::string &host,
				 const std::string &filesPath);
  /**
   * Create a time_t from the Time_t struct in the params class
   * @parm[in] p  The struct
   * @return the time
   */

  time_t _timeFromParams(const Params::Time_t &p) const;

  /**
   * Create and return a list of files within a time range
   * @param[in] path  Place to search for files
   * @param[in] t0  Earliest time
   * @param[in] t1  Latest time
   * @param[out] files  The file names
   *
   * @return true if able to get some files onto the vector
   */

  bool _identifyFiles(const std::string &host,
                      const std::string &path,
		      const time_t &t0,
                      const time_t &t1,
		      std::vector<string> &files) const;

};


#endif
