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
 * Class controlling the command line arguments for this program.
 *  
 * @date 10/31/2008
 *
 */

#ifndef Args_HH
#define Args_HH

#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#include <tdrp/tdrp.h>

using namespace std;

/** 
 * @class Args
 */

class Args
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief TDRP overrides specified in the command line arguments.
   */

  tdrp_override_t override;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   * @param[in] prog_name Program name.
   */

  Args(int argc, char **argv, char *prog_name);
  

  /**
   * @brief Destructor
   */

  ~Args(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the list of input files to process.
   *
   * @return Returns the list of files to process.
   */

  const vector< string > &getFileList(void)
  {
    return _inputFileList;
  }
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief The program name for error messages.
   */

  string _progName;

  /**
   * @brief The list of input files to process.
   */

  vector< string > _inputFileList;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Read the file containing the list of input files to process.
   *
   * @param[in] file_name The name of the file containing the list of
   *                      input files.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readFileListFile(const string &file_name);
  

  /**
   * @brief Print the usage for this program.
   *
   * @param[in,out] stream Stream to use for printing.
   */

  void _usage(FILE *stream);
  
};


#endif
