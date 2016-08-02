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
/******************************************************************************
 *  MDV_CHUNK.C  Subroutines for manipulating known types of MDV chunks.
 *               Currently, these routines are local to the MDV library
 *               routines and, so, are prototyped in mdv_private.h.
 *  N. Rehak.  Jul 1996. RAP.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <rapformats/dobson.h>
#include <toolsa/mem.h>

#include "mdv_private.h"


/******************************************************************************
 * DOBSON_VOL_PARAMS_FROM_BE:  Convert the fields in Dobson volume params chunk
 * data from big endian format to native format.
 */
 
void dobson_vol_params_from_BE(vol_params_t *vol_params)
{

  int nbytes_char;
  
  BE_to_array_32((ui32 *) &vol_params->file_time,
		 N_VOL_PARAMS_TIMES * sizeof(radtim_t));
  
  nbytes_char = N_RADAR_PARAMS_LABELS * R_FILE_LABEL_LEN;
  
  BE_to_array_32((ui32 *) &vol_params->radar,
		 sizeof(radar_params_t) - nbytes_char);
  
  nbytes_char = N_CART_PARAMS_LABELS * R_FILE_LABEL_LEN;
  
  BE_to_array_32((ui32 *) &vol_params->cart,
		 sizeof(cart_params_t) - nbytes_char);
  
  BE_to_array_32((ui32 *) &vol_params->nfields,
		 sizeof(si32));
  
  return;

}

/******************************************************************************
 * DOBSON_VOL_PARAMS_TO_BE:  Convert the fields in Dobson volume params chunk
 * data from native format to big endian format.
 */
 
void dobson_vol_params_to_BE(vol_params_t *vol_params)
{

  int nbytes_char;
  
  BE_from_array_32((ui32 *) &vol_params->file_time,
		   N_VOL_PARAMS_TIMES * sizeof(radtim_t));
  
  nbytes_char = N_RADAR_PARAMS_LABELS * R_FILE_LABEL_LEN;

  vol_params->radar.nbytes_char = nbytes_char;
  BE_from_array_32((ui32 *) &vol_params->radar,
		   sizeof(radar_params_t) - nbytes_char);
  
  nbytes_char = N_CART_PARAMS_LABELS * R_FILE_LABEL_LEN;
  
  vol_params->cart.nbytes_char = nbytes_char;
  BE_from_array_32((ui32 *) &vol_params->cart,
		   sizeof(cart_params_t) - nbytes_char);
  
  BE_from_array_32((ui32 *) &vol_params->nfields,
		   sizeof(si32));

  return;

}

/******************************************************************************
 * DOBSON_ELEVATIONS_FROM_BE:  Convert the fields in Dobson elevations chunk
 * data from big endian format to native format.
 */
 
void dobson_elevations_from_BE(void *elevations, long size)
{
  /*
   * Convert the data to native format.
   */

  BE_to_array_32((si32 *)elevations, size);
  
  return;
}

/******************************************************************************
 * DOBSON_ELEVATIONS_TO_BE:  Convert the fields in Dobson elevations chunk
 * data from native format to big endian format.
 */
 
void dobson_elevations_to_BE(void *elevations, long size)
{
  /*
   * Convert the data to big endian format.
   */

  BE_from_array_32((si32 *)elevations, size);
  
  return;
}


