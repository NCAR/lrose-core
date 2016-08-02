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
/*********************************************************************
 * read_volparams.c
 *
 * reads a vol params packet
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_slave.h"

int read_volparams(void)

{

  int vol_params_read = FALSE;
  si32 nbytes_char;
  si32 nelev;
  si32 nheights;

  si32 *radar_elevations;
  si32 *plane_heights;
  si32 packet_id;
  
  vol_params_t *vol_params;
  
  /*
   * read in a packet
   */
  
  while (!vol_params_read) {
  
    if (read_packet(&packet_id) == CS_FAILURE) {
      return(CS_FAILURE);
    }
    
    /*
     * check for volume packet type
     */
    
    if (packet_id == VOL_PARAMS_PACKET_CODE) {
      
      vol_params = (vol_params_t *) Glob->packet;

      umalloc_verify();

      /*
       * decode the data from network byte order
       */

      BE_to_array_32((ui32 *) &vol_params->mid_time,
		     (ui32) sizeof(radtim_t));
      
      nbytes_char =
	(si32) BE_to_si32((ui32) vol_params->radar.nbytes_char);
      
      BE_to_array_32((ui32 *) &vol_params->radar,
		     (ui32) (sizeof(radar_params_t) - nbytes_char));
      
      nbytes_char =
	(si32) BE_to_si32((ui32) vol_params->cart.nbytes_char);
      
      BE_to_array_32((ui32 *) &vol_params->cart,
		     (ui32) (sizeof(cart_params_t) - nbytes_char));
      
      vol_params->nfields = (si32) BE_to_si32((ui32) vol_params->nfields);
      
      umalloc_verify();

      radar_elevations = (si32 *)
	((char *) vol_params + sizeof(vol_params_t));
      
      nelev = vol_params->radar.nelevations;

      BE_to_array_32((ui32 *) radar_elevations,
		     (ui32) (nelev * sizeof(si32)));
      
      plane_heights = radar_elevations + nelev;
      
      nheights = vol_params->cart.nz * N_PLANE_HEIGHT_VALUES;

      umalloc_verify();

      BE_to_array_32((ui32 *) plane_heights,
		     (ui32) (nheights * sizeof(si32)));
      
      /*
       * place this data in shared memory
       */
      
      umalloc_verify();

      setup_volume(vol_params, radar_elevations, plane_heights);
      
      vol_params_read = TRUE;
      
    } /* if (packet_id == VOL_PARAMS_PACKET_CODE) */
    
  } /* while (!vol_params_read) */
  
  return(CS_SUCCESS);
  
}
