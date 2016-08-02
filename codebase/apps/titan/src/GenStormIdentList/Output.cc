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
//////////////////////////////////////////////////////////
// Sample.cc : data time output
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////
//
// Reads in the storm file.
//
// Writes out start and end times in format suitable for
// scripts.
//
//////////////////////////////////////////////////////////a

#include "Params.hh"
#include "Output.hh"
#include <toolsa/str.h>
using namespace std;

//////////////
// Constructor

Output::Output (char *prog_name,
		Params *params,
		char *input_file_path,
		storm_file_handle_t *s_handle)
  
{
  
  // set data members
  
  _progName = STRdup(prog_name);
  _params = params;
  _inputFilePath = STRdup(input_file_path);
  _s_handle = s_handle;

}

/////////////
// destructor

Output::~Output ()
  
{

  STRfree(_inputFilePath);
  STRfree(_progName);

}

////////////////
// process a file

int Output::Process()

{

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Processing %s\n", _inputFilePath);
  }

  // open storm properties files
  
  if (RfOpenStormFiles (_s_handle, "r", _inputFilePath,
			(char *) NULL, "Output::Process")) {
    return(-1);
  }

  // read in storm properties file header

  if (RfReadStormHeader(_s_handle, "Output::Process") != R_SUCCESS) {
    return (-1);
  }

  // get start and end time

  date_time_t start_time;
  date_time_t end_time;

  start_time.unix_time = _s_handle->header->start_time;
  uconvert_from_utime(&start_time);

  end_time.unix_time = _s_handle->header->end_time;
  uconvert_from_utime(&end_time);

  // print output line

  if (_params->make_old_list) {
    fprintf(stdout, "storm_ident -params %s "
	    "-starttime %.4d/%.2d/%.2d_%.2d:%.2d:%.2d "
	    "-endtime %.4d/%.2d/%.2d_%.2d:%.2d:%.2d\n",
	    _params->storm_ident_params_path,
	    start_time.year, start_time.month, start_time.day, 
	    start_time.hour, start_time.min, start_time.sec,
	    end_time.year, end_time.month, end_time.day, 
	    end_time.hour, end_time.min, end_time.sec);
  } else {
    fprintf(stdout, "StormIdent -params %s "
	    "-start \"%.4d %.2d %.2d %.2d %.2d %.2d\" "
	    "-end \"%.4d %.2d %.2d %.2d %.2d %.2d\"\n",
	    _params->storm_ident_params_path,
	    start_time.year, start_time.month, start_time.day, 
	    start_time.hour, start_time.min, start_time.sec,
	    end_time.year, end_time.month, end_time.day, 
	    end_time.hour, end_time.min, end_time.sec);
  }

  // close storm files

  RfCloseStormFiles(_s_handle, "Output::Process");

  return (0);

}

 
