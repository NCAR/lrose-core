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
/**
 * @file Args.hh
 * @brief Command line object
 * @class Args
 * @brief Command line object
 */

#ifndef ARGS_HH
#define ARGS_HH

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
  Args(void);

  /**
   * Destructor
   */
  ~Args(void);

  /**
   * Parse the args to set state
   * @param[in] argc
   * @param[in] argv
   * @param[in] prog_name
   *
   * @return 0 for o.k.
   */
  int parse(int argc, char **argv, std::string &prog_name);

  /**
   * public data
   */

  tdrp_override_t override;

  bool _isArchive;
  time_t _archiveT0;
  time_t _archiveT1;

  bool _isFilelist;
  vector<string> inputFileList;

protected:
  
private:

  void _usage(std::string &prog_name, std::ostream &out);
  
};

#endif

