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
//
// Mdv2MatlabParams.cc : TDRP parameters
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
//////////////////////////////////////////////////////////

#include <stdio.h>

#include <toolsa/os_config.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>

#include "Mdv2MatlabParams.h"

/**************************************************************
 * Constructor
 */

Mdv2MatlabParams::Mdv2MatlabParams(char *params_file_path,
			       tdrp_override_t *override,
			       char *prog_name,
			       int check_params,
			       int print_params)
{
  okay = TRUE;
  done = FALSE;
  
  _table = mdv2matlab_tdrp_init(&params);

  if (TDRP_read(params_file_path, _table, &params,
		override->list) == FALSE)
  {
    fprintf(stderr, "ERROR - %s:Mdv2MatlabParams::Mdv2MatlabParams\n", prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);

    okay = FALSE;
    done = TRUE;
    return;
  }
  TDRP_free_override(override);

  if (check_params)
  {
    TDRP_check_is_set(_table, &params);
    done = TRUE;
    return;
  }

  if (print_params)
  {
    TDRP_print_params(_table, &params, prog_name, TRUE);
    done = TRUE;
    return;
  }
  
  // set malloc debug level

  if (params.malloc_debug_level > 0)
    umalloc_debug(params.malloc_debug_level);

  return;
}




