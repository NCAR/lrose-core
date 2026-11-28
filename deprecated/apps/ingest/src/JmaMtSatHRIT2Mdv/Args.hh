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
 * @date 1/30/2012
 *
 */

#ifndef Args_HH
#define Args_HH

#include <cstdio>
#include <string>
#include <vector>

#include <tdrp/tdrp.h>

using namespace std;


/** 
 * @class Args
 */

class Args
{
 public:

  /**
   * @brief TDRP overrides specified in the command line arguments.
   */

  tdrp_override_t override;
  
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
   * @brief Get the list of input files from the command line.
   *
   * @return Returns the list of input files.
   */

  const vector< string > &getFileList()
  {
    return _fileList;
  }
  

 private:

  /**
   * @brief The program name for error messages.
   */

  string _progName;
  
  /**
   * @brief The list of input files from the command line.
   */

  vector< string > _fileList;
  
  /**
   * @brief Print the usage for this program.
   */

  void _usage(FILE *stream);
  
};


#endif
