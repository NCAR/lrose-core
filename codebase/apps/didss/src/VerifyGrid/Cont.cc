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
///////////////////////////////////////////////////////////////
// Cont.cc
//
// Contingency grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Cont.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
using namespace std;

//////////////
// Constructor

Cont::Cont(char *prog_name, Params *params) : Comps(prog_name, params)

{

  // initialize contingency table
  
  memset (&_cont, 0, sizeof(contingency_t));

  // scan contingency file

  _scanContFile = NULL;
  _scanContDate = 0;
  
}

/////////////
// destructor

Cont::~Cont()

{

  // close cont file

  if (_scanContFile != NULL) {
    fclose(_scanContFile);
    _scanContFile = NULL;
  }
    
}

/////////////////
// _openContFile()
//
// Returns 0 on success, -1 on failure
//

int Cont::_openContFile(time_t timeCent)
  
{
  
  UTIMstruct time_struct;
  UTIMunix_to_date(timeCent, &time_struct);

  /*
   * compute vol date
   */

  int vol_date = (time_struct.year * 10000 +
		  time_struct.month * 100 +
		  time_struct.day);

  if (vol_date != _scanContDate) {
    
    if (_scanContFile != NULL) {
      fclose(_scanContFile);
      _scanContFile = NULL;
    }
    
    _scanContDate =  vol_date;

    char file_path[MAX_PATH_LEN];

    sprintf(file_path, "%s%s%d.%s",
	    _params->scan_cont_dir, PATH_DELIM,
	    vol_date, _params->scan_cont_ext);

    if ((_scanContFile =
	 fopen(file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:Cont::update\n", _progName);
      fprintf(stderr, "Cannot open scan stats file for writing\n");
      perror(file_path);
      return(-1);
    }
    
    fprintf(_scanContFile,
	    "year month day hour min sec "
	    "lead_time(mins) threshold(dBZ) "
	    "n_measured n_forecast n_success "
	    "n_failure n_false_alarm n_non_event"
	    "\n");
    
  }
  
  return (0);

}
