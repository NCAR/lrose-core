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
/*********************************************************************
 * set_derived_params.c
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Alenia2Mom.h"
using namespace std;

void set_derived_params(void)

{

  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);

  if (Glob->params.output_format == LL_FORMAT) {
    Glob->params.output_vel = pTRUE;
    Glob->params.output_width = pTRUE;
    Glob->params.output_zdr = pTRUE;
  }

  Glob->nfields_out = 0;

  if (Glob->params.output_dbz) {
    Glob->nfields_out++;
  }

  if (Glob->params.output_vel) {
    Glob->nfields_out++;
  }

  if (Glob->params.output_width) {
    Glob->nfields_out++;
  }

  if (Glob->params.output_zdr) {
    Glob->nfields_out++;
  }

  if (Glob->params.output_format == LL_FORMAT) {
    if (Glob->nfields_out != LL_NFIELDS) {
      fprintf(stderr,
	      "ERROR - %s:set_derived_params\n", Glob->prog_name);
      fprintf(stderr,
	      "Lincoln Format can only have %d fields\n", LL_NFIELDS);
      fprintf(stderr, "Data only seems to have %d fields\n",
	      Glob->nfields_out);
      tidy_and_exit(-1);
    }
  }

}


