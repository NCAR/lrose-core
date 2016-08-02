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
// Params.cc : TDRP parameters
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
//////////////////////////////////////////////////////////

#include "Params.h"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

// Declare static instance of Params

Params::Params (char *params_file_path,
		tdrp_override_t *override,
		char *prog_name,
		int check_params,
		int print_params,
		int print_short)

{

  OK = TRUE;
  Done = FALSE;
  
  table = OverlayCreate_tdrp_init(&p);

  if (FALSE == TDRP_read(params_file_path, table, &p,
			 override->list)) {
    fprintf(stderr, "ERROR - %s:Parms::Params\n", prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    OK = FALSE;
    Done = TRUE;
    return;
  }
  TDRP_free_override(override);

  if (check_params) {
    TDRP_check_is_set(table, &p);
    Done = TRUE;
    return;
  }

  if (print_params) {
    TDRP_print_params(table, &p, prog_name, TRUE);
    Done = TRUE;
    return;
  } else if (print_short) {
    TDRP_print_params(table, &p, prog_name, FALSE);
    Done = TRUE;
    return;
  }
  
  // set malloc debug level

  if (p.malloc_debug_level > 0) {
    umalloc_debug(p.malloc_debug_level);
  }

  return;

}




