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
 * @file Args.hh
 *
 * @class Args
 *
 * Class controlling the command line arguments for the program.
 *  
 * @date 8/30/2011
 *
 */

#ifndef Args_HH
#define Args_HH

#include <stdio.h>
#include <string>
#include <time.h>

#include <tdrp/tdrp.h>

using namespace std;


/** 
 * @class Args
 */

class Args
{
 public:

  /**
   * @brief Constructor
   */

  Args(int argc, char **argv, char *prog_name);
  
  /**
   * @brief Destructor
   */

  ~Args(void);
  
  /**
   * @brief Get the user-entered archive start time.
   *
   * @return Returns the user-entered archive start time, or 
   *         DateTime::NEVER if the user didn't enter a start time.
   */

  DateTime getStartTime() const 
  {
    return _startTime;
  }
  
  /**
   * @brief Get the user-entered archive end time.
   *
   * @return Returns the user-entered archive end time, or 
   *         DateTime::NEVER if the user didn't enter a end time.
   */

  DateTime getEndTime() const 
  {
    return _endTime;
  }
  

  /**
   * @brief TDRP overrides specified in the command line arguments.
   */

  tdrp_override_t override;
  

 private:

  /**
   * @brief The program name for error messages
   */

  string _progName;
  

  /**
   * @brief Archive start time.
   */

  DateTime _startTime;
  
  /**
   * @brief Archive end time.
   */

  DateTime _endTime;
  
  /**
   * @brief Print the usage for this program.
   *
   * @param[in] stream The stream to use for printing.
   */

  void _usage(FILE *stream);
  
};


#endif
