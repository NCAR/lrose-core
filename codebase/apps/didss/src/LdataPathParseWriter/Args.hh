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
 * @file Args.hh
 * @brief Argument handler
 * @class Args
 * @brief Argument handler
 */
#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>
#include <iostream>
#include <ctime>

class Args
{
  
public:

  /**
   * Constructor
   */
  Args(void);

  /**
   * Destructor
   */
  ~Args(void);

  /**
   * Parse input args and set internal state
   *
   * @param[in] argc
   * @param[in] argv
   * @param[out] prog_name  
   *
   * @return 0 for good, -1 for bad
   */
  int parse(int argc, char **argv, std::string &prog_name);

  std::string _path;       /**< The full path */
  std::string _suffix;     /**< The suffix for the path */
  std::string _dataType;   /**< Data type (mdv, nc, ...) */
  std::string _fileExt;    /**< File extension (mdv, nc, ...) */
  bool _isFcast;           /**< True if path is for forecast data */
  bool _debug;             /**< True for debugging */

protected:
private:

  /**
   * Print out usage statement
   * @param[in] prog_name
   * @param[in] out Where to print 
   */
  void _usage(std::string &prog_name, std::ostream &out);
  
};

#endif

