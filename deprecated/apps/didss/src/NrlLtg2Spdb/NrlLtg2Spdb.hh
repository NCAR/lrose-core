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
/************************************************************************
 * NrlLtg2Spdb.hh : header file for the NrlLtg2Spdb program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Dan Megenhardt
 *
 ************************************************************************/

#ifndef NrlLtg2Spdb_HH
#define NrlLtg2Spdb_HH

#include <toolsa/MemBuf.hh>
#include <rapformats/ltg.h>
#include <didss/DsInputPath.hh>
#include <Spdb/DsSpdb.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class NrlLtg2Spdb
{

 public:

  // constructor
  
  NrlLtg2Spdb(int argc, char **argv);
  
  // Destructor
  
  ~NrlLtg2Spdb();
  
  // Run the program.

  int Run();
  
  // Flag indicating whether the program status is currently okay.

  bool isOK;
  
 private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // Input objects

  DsInputPath *_inputPath;
  date_time_t input_file_time;

  // Output objects

  MemBuf _spdbBuf;
  DsSpdb _spdb;

  void _sendSpdbData(LTG_strike_t *strike_buffer,
		     int num_strikes);

  LTG_strike_t *NRLLTG_read_strike(FILE *input_file, char *prog_name);
  
  static const char *_className(void) { return("NrlLtg2Spdb"); }
  
};


#endif
