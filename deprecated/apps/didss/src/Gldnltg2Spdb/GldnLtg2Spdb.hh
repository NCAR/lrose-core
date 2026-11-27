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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/************************************************************************
 * GldnLtg2Spdb.hh : header file for the GldnLtg2Spdb program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Gary Blackburn/Kay Levesque
 *
 ************************************************************************/

#ifndef GldnLtg2Spdb_HH
#define GldnLtg2Spdb_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <didss/DsInputPath.hh>
#include <rapformats/GldnLtgFile.hh>
using namespace std;

class GldnLtg2Spdb
{
 public:

  // Destructor

  ~GldnLtg2Spdb(void);
  
  // Get GldnLtg2Spdb singleton instance

  static GldnLtg2Spdb *Inst(int argc, char **argv);
  static GldnLtg2Spdb *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  // Constructor -- private because this is a singleton object

  GldnLtg2Spdb(int argc, char **argv);
  
  // Singleton instance pointer

  static GldnLtg2Spdb *_instance;
  
  // Program parameters.

  string _progName;
  Args *_args;
  Params *_params;
  
  // Input objects

  DsInputPath *_input;
  
  // Latest data time -- used for speeding up processes restarts by not
  // reprocessing existing data.

  time_t _beginLatestDataTime;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GldnLtg2Spdb");
  }
  
};


#endif
