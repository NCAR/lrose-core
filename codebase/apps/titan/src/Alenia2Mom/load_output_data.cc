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
/***************************************************************************
 * load_output_data.c
 *
 * Loads output data buffer.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
using namespace std;

void load_output_data(alenia_params_t *al_params,
		      ui08 *input_data,
		      ui08 *output_data)

{

  char *cout;
  ui08 *out;
  ui08 *in;

  int i;
  int ngates = al_params->ngates;
  int nfields = Glob->nfields_out;
  int field_offset;

  /*
   * zero out write buffer
   */

  memset(output_data, 0, ngates * nfields);
  
  /*
   * initialize
   */

  in = input_data;
  field_offset = 0;

  /*
   * load up dbz as applicable
   */

  if (Glob->params.output_dbz) {

    if (al_params->dbz_avail) {

      /*
       * if required, rescale dbz input data
       */
      
      if (Glob->params.rescale_dbz) {
	rescale_dbz(in, ngates);
      }

      out = output_data + field_offset;
      for (i = 0; i < ngates; i++, in++, out += nfields) {
	*out = *in;
      }
    }
    field_offset++;

  } else {

    if (al_params->dbz_avail) {
      in += ngates;
    }

  }

  /*
   * load up zdr as applicable
   */

  if (Glob->params.output_zdr) {
    if (al_params->zdr_avail) {
      out = output_data + field_offset;
      for (i = 0; i < ngates; i++, in++, out += nfields) {
	*out = *in;
      }
    }
    field_offset++;
  } else {
    if (al_params->zdr_avail) {
      in += ngates;
    }
  }

  /*
   * load up vel as applicable
   */

  if (Glob->params.output_vel) {

    if (al_params->vel_avail) {
      
#ifdef CORRECT_VEL
      
      out = output_data + field_offset;
      for (i = 0; i < ngates; i++, in++, out += nfields) {
	*out = *in;
      }
      
#else

      /*
       * This code is put in to alter the velocity data.
       * The vel bytes should range from 0 for max neg velocity
       * to 255 for max pos velocity. Instead, it seems that
       * a byte val of 0 represents vel of 0, byte val 127 represents
       * max pos vel, byte val 128 represents max neg velocity
       * and byte val 255 represents just less than 0.
       * Therefore, we need to add 128 to the lower 127 vals,
       * and subtract 128 from the upper 128 values.
       */
      
      cout = (char  *) output_data + field_offset;
      for (i = 0; i < ngates; i++, in++, cout += nfields) {
	if (*in > 127) {
	  *cout = *in - 127;
	} else {
	  *cout = *in + 128;
	}
      }

#endif

    }
    field_offset++;
  } else {
    if (al_params->vel_avail) {
      in += ngates;
    }
  }

  /*
   * load up width as applicable
   */

  if (Glob->params.output_width) {
    if (al_params->width_avail) {
      out = output_data + field_offset;
      for (i = 0; i < ngates; i++, in++, out += nfields) {
	*out = *in;
      }
    }
    field_offset++;
  } else {
    if (al_params->width_avail) {
      in += ngates;
    }
  }
	 
}


