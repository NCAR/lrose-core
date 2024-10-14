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
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <tdrp/tdrp.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Args {
  
public:

  // constructor

  Args (const string &prog_name);

  // destructor

  virtual ~Args ();

  // parse command line
  // Returns 0 on success, -1 on failure

  int parse(const int argc, const char **argv);

  // get the legacy params file from the command line
  // returns 0 on success, -1 on failure

  int getLegacyParamsPath(const int argc, const char **argv,
                          string &legacyPath);

  // get the tdrp params file from the command line
  // returns 0 on success, -1 on failure

  int getTdrpParamsPath(const int argc, const char **argv,
                        string &tdrpPath);

  // get the print mode from the command line
  // returns 0 on success, -1 on failure
  
  int getTdrpPrintMode(const int argc, const char **argv,
                       tdrp_print_mode_t &printMode);

  // kay-value pairs for overriding TDRP params

  tdrp_override_t override;

  // get methods
  
  bool usingLegacyParams() const { return _usingLegacyParams; }
  string legacyParamsPath() const { return _legacyParamsPath; }


protected:
  
private:

  string _progName;

  bool _usingLegacyParams;
  string _legacyParamsPath;

  int _processLegacyArgs(int argc, const char **argv);
  void _usage(ostream &out);
  
};

#endif
