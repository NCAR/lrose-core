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
 * @date 7/19/2011
 *
 */

#ifndef Args_HH
#define Args_HH

#include <cstdio>
#include <string>
#include <vector>

#include <toolsa/DateTime.hh>

using namespace std;


/** 
 * @class Args
 */

class Args
{
 public:

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   * @param[in] prog_name Program name used in output messages.
   */

  Args(int argc, char **argv, char *prog_name);
  
  /**
   * @brief Destructor
   */

  ~Args(void);
  

  /**
   * @brief Get the name of the file to process.
   *
   * @return Returns the name of the file to process.
   */

  inline string getFileName() const
  {
    return _fileName;
  }
  
  /**
   * @brief Get the flag indicating whether to print the angle information.
   *
   * @return Returns the flag indicating whether to print the angle
   *         information.
   */

  inline bool isPrintAngles() const
  {
    return _printAngles;
  }
  
  /**
   * @brief Get the flag indicating whether to print the data.
   *
   * @return Returns the flag indicating whether to print the data.
   */

  inline bool isPrintData() const
  {
    return _printData;
  }
  
  /**
   * @brief Get the start time for printing.
   *
   * @return Returns the start time for printing or DateTime::NEVER if no
   *         start time was specified.
   */

  inline DateTime getStartTime() const
  {
    return _startTime;
  }
  
  /**
   * @brief Get the end time for printing.
   *
   * @return Returns the end time for printing or DateTime::NEVER if no
   *         end time was specified.
   */

  inline DateTime getEndTime() const
  {
    return _endTime;
  }
  
  
 private:

  /**
   * @brief The program name for error messages.
   */

  string _progName;
  
  /**
   * @brief The name of the file to process.
   */

  string _fileName;
  
  /**
   * @brief Flag indicating to print the angle information from the
   *        file.
   */

  bool _printAngles;
  
  /**
   * @brief Flag indicating to print the data from the file.
   */

  bool _printData;
  
  /**
   * @brief The start time for the data.  Set to DateTime::NEVER if printing
   *        should start at the beginning of the file.
   */

  DateTime _startTime;
  
  /**
   * @brief The end time for the data.  Set to DateTime::NEVER if printing
   *        should continue through the end of the file.
   */

  DateTime _endTime;
  
  /**
   * @brief Print the usage for this program.
   */

  void _usage(FILE *stream);
  
};


#endif
