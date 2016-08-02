/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*****************************************************************************
 * update_params.c
 *
 * set up the shared memory header and buffer, and load in the
 * relevant parameters
 *
 * Returns 1 if params have changed, 0 otherwise
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 * Copied from rdata_to_socket for use in ufnexrad2gate by Nancy Rehak,
 * Mar 1995.
 ****************************************************************************/

#include "ufnexrad2gate.h"
#include <dataport/bigend.h>

int update_params(ui08 *param_buffer,
		  ui08 *gate_params_pkt,
		  si32 nbytes_params_pkt)

{

  static int first_call = TRUE;
  static si32 nbytes_fparams;

  int params_have_changed = FALSE;

  static gate_data_radar_params_t rparams;
  static gate_data_field_params_t *fparams;

  static gate_data_radar_params_t prev_rparams;
  static gate_data_field_params_t *prev_fparams;

  gate_data_radar_params_t *pkt_rparams;
  gate_data_field_params_t *pkt_fparams;


  if (first_call)
  {
    /*
     * allocate field params, and initialize
     */

    nbytes_fparams = Glob->params.file_info.len *
      sizeof(gate_data_field_params_t);

    fparams = (gate_data_field_params_t *) umalloc
      ((ui32) nbytes_fparams);

    prev_fparams = (gate_data_field_params_t *) umalloc
      ((ui32) nbytes_fparams);

    memset ((void *)  &prev_rparams,
            (int) 0, (size_t) sizeof(gate_data_radar_params_t));
    
    memset ((void *)  prev_fparams,
            (int) 0, (size_t) nbytes_fparams);

  } /* if (first_call) */

  /*
   * set radar params
   */

  memcpy((void *)&rparams, (void *)param_buffer,
	 (size_t)sizeof(gate_data_radar_params_t));
  
  /*
   * set field params
   */

  memcpy((void *)fparams,
	 (void *)(param_buffer + sizeof(gate_data_radar_params_t)),
	 nbytes_fparams);
  
  /*
   * check to see if params have changed
   */

  params_have_changed = FALSE;

  if (memcmp((void *) &rparams, (void *) &prev_rparams,
	     (size_t) sizeof(gate_data_radar_params_t)))
    params_have_changed = TRUE;


  if (memcmp((void *) fparams, (void *) prev_fparams,
	     (size_t) nbytes_fparams))
    params_have_changed = TRUE;


  if (params_have_changed)
  {
    /*
     * set local pointers
     */
    
    pkt_rparams = (gate_data_radar_params_t *) gate_params_pkt;
    pkt_fparams = (gate_data_field_params_t *)
      (gate_params_pkt + sizeof(gate_data_radar_params_t));

    /*
     * copy params to pkt areas
     */

    memcpy ((void *) pkt_rparams,
            (void *) &rparams,
            (size_t) sizeof(gate_data_radar_params_t));

    memcpy ((void *) pkt_fparams,
            (void *) fparams,
            (size_t) nbytes_fparams);

    /*
     * store in prev locations for later comparison
     */

    memcpy ((void *) &prev_rparams,
            (void *) &rparams,
            (size_t) sizeof(gate_data_radar_params_t));

    memcpy ((void *) prev_fparams,
            (void *) fparams,
            (size_t) nbytes_fparams);

    if (Glob->params.debug)
      printf("params have changed\n");

    /*
     * code the params into network byte order
     */
    
    BE_from_array_32((ui32 *) gate_params_pkt,
		     (ui32) nbytes_params_pkt);
    
  } /* if (params_have_changed) */
  
  first_call = FALSE;

  return (params_have_changed);

}
