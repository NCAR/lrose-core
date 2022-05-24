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
// Cases.cc : Case handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#include "Cases.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <cassert>
using namespace std;

//////////////
// Constructor

Cases::Cases (const char *prog_name,
	      int debug,
	      char *case_file_path)

{

  // initialize

  _progName = STRdup(prog_name);
  _debug = debug;
  _caseFilePath = STRdup(case_file_path);

  OK = TRUE;

  // read in case file
  
  init_case_track_handle(&_caseHandle, _progName, _debug);

  if (read_case_track_file(&_caseHandle, _caseFilePath)) {
    fprintf(stderr, "ERROR - %s:Cases::Cases\n", _progName);
    fprintf(stderr, "Cannot read in case track file\n");
    free_case_track_handle(&_caseHandle);
    OK = FALSE;
    return;
  }
  reset();
  
}

/////////////
// Destructor

Cases::~Cases()

{

  // free up memory

  STRfree(_progName);
  STRfree(_caseFilePath);

  // free up case handle

  free_case_track_handle(&_caseHandle);

}

////////////////////////////////////////
// reset()
//
// Set back to case 0
//
////////////////////////////////////////

void Cases::reset()
  
{
  _caseNum = 0;
}

////////////////////////////////////////
// next()
//
// get next case
//
// returns 0 on success, -1 on failure
////////////////////////////////////////

int Cases::next(case_track_t *this_case)
  
{

  if (_caseNum >= _caseHandle.n_cases) {
    return (-1);
  } else {
    *this_case = _caseHandle.cases[_caseNum++];
    return (0);
  }

}

