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
// FileLock.cc
//
// FileLock object - file locking to make sure only one
// instance of the program runs per storm_data_dir
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "FileLock.hh"
#include <toolsa/str.h>
#include <toolsa/file_io.h>
using namespace std;

//////////////
// constructor
//

FileLock::FileLock(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  
  sprintf(_lockFilePath, "%s%s%s.%s",
	  _params->storm_data_dir, PATH_DELIM, _progName, "lock");

  if (ta_makedir_recurse(_params->storm_data_dir)) {
    fprintf(stderr, "ERROR - %s:Filelock::Filelock\n", _progName);
    fprintf(stderr, "  Cannot create dir: '%s'\n", _params->storm_data_dir);
    OK = FALSE;
    return;
  }

  _fd = ta_create_lock_file(_lockFilePath);
  
  if (_fd == NULL) {
    fprintf(stderr, "ERROR - %s:Filelock::Filelock\n", _progName);
    fprintf(stderr, "Cannot create lock file '%s'\n", _lockFilePath);
    fprintf(stderr, "Check that dir %s exists\n", _params->storm_data_dir);
    fprintf(stderr, "%s may already be running in this directory\n",
	    _progName);
    OK = FALSE;
  }

}

/////////////
// destructor
//

FileLock::~FileLock()

{

  if (_fd != NULL) {
    ta_remove_lock_file(_lockFilePath, _fd);
  }
  STRfree(_progName);

}


