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
/////////////////////////////////////////////////////////////
// Args.hh: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
/////////////////////////////////////////////////////////////

/**
 * @file Args.hh
 * @brief Command line object
 * @class Args
 *
 * An object to read and parse command line arguments
 */

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>
#include <iostream>
#include <tdrp/tdrp.h>
using namespace std;

class Args {
  
public:

  /**
   * Constructor
   */
  Args();

  /**
   * Destructor
   */
  ~Args();

  /**
   * Parse the command line arguments
   * @param[in] argc The number of command line arguments
   * @param[in] argv The list of command line arguments
   * @param[in] prog_name The name of the application, for help and
   *            debugging messages
   * @return 0 on success, -1 if an error occcurs
   */
  int parse(int argc, char **argv, string &prog_name);

  tdrp_override_t override;  /**< Object used to override application parameters with command line arguments */
  time_t startTime;          /**< The start time to process (if provided on command line) */
  time_t endTime;            /**< The end time to process (if provided on command line) */
  vector<string> inputFileList;  /**< A list of files to process */

  /**
   * Print application usage message to the supplied stream
   * @param[in] The name of the application
   * @param[out] The stream to print to
   */
  void usage(string &prog_name, ostream &out);
  
protected:
  
private:

};

#endif
