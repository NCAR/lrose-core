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
 * @file RadxAppArgs.hh
 * @brief Command line argument handler
 * @class RadxAppArgs
 * @brief Command line argument handler
 */

#ifndef RADX_APP_ARGS_HH
#define RADX_APP_ARGS_HH

#include <tdrp/tdrp.h>
#include <string>
#include <vector>
#include <iostream>

class RadxAppArgs {
  
public:

  /**
   * Constructor
   */
  RadxAppArgs(void);

  /**
   * Destructor
   */
  virtual ~RadxAppArgs(void);
  
  /**
   * parse the command line
   * @param[in] argc
   * @param[in] argv
   * @param[out] prog_name
   *
   * @return 0 on success, -1 on failure
   */
  int parse (int argc, char **argv, std::string &prog_name);

  tdrp_override_t override;    /**< TDRP overrides */
  time_t startTime;            /**< Archive mode start time */
  time_t endTime;             /**< Archive mode end time */
  std::vector<std::string> inputFileList; /**< file list mode files*/
  bool tdrpExit;    /**< True to exit after reading TDRP */

protected:
private:

  std::string _progName;           /**< Program name */
  void _usage(std::ostream &out);  /**< Usage statement */
  
};

#endif



