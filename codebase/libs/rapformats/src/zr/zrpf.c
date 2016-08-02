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
 * zrpf.c
 *
 * Routines for handling ZR point forecast data
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * Nov 1997
 *
 *********************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <dataport/bigend.h>
#include <rapformats/zrpf.h>
#include <assert.h>

static void set_buffer_ptrs(zrpf_handle_t *handle);
     
/****************************
 * zrpf_handle_init
 *
 * Initializes handle
 */

void zrpf_handle_init(zrpf_handle_t *handle)
     
{
  
  if (handle->init_flag != ZRPF_HANDLE_INIT_FLAG) {
    handle->mbuf = MEMbufCreate();
    handle->init_flag = ZRPF_HANDLE_INIT_FLAG;
    zrpf_handle_alloc(handle, 0);
  }

}

/****************************
 * zrpf_handle_free
 *
 * Frees handle
 */

void zrpf_handle_free(zrpf_handle_t *handle)

{

  MEMbufDelete(handle->mbuf);
  handle->init_flag = 0;
  
}

/****************************
 * zrpf_handle_alloc
 *
 * Allocates buffer space
 */

void zrpf_handle_alloc(zrpf_handle_t *handle,
		       int n_forecasts)
     
{

  int nbytes_needed;
  
  assert(handle->init_flag == ZRPF_HANDLE_INIT_FLAG);
  
  nbytes_needed =
    sizeof(zrpf_hdr_t) + sizeof(zr_params_t) +
    n_forecasts * sizeof(zrpf_precip_t);
  
  MEMbufPrepare(handle->mbuf, nbytes_needed);

  set_buffer_ptrs(handle);

}

/****************************
 * zrpf_handle_to_BE
 *
 * Swaps in place
 */

void zrpf_handle_to_BE(zrpf_handle_t *handle)

{

  assert(handle->init_flag == ZRPF_HANDLE_INIT_FLAG);

  BE_from_array_32(handle->point_forecast,
		   (handle->hdr->n_forecasts *
		    sizeof(zrpf_precip_t)));

  zr_params_to_BE(handle->zr_params);

  BE_from_array_32(handle->hdr, ZRPF_HDR_NBYTES_32);

  
  
}

/****************************
 * zrpf_handle_load_from_chunk
 *
 * 1. Loads up handle buffer from chunk.
 * 2. Sets pointers.
 * 3. Swaps from BE.
 */

void zrpf_handle_load_from_chunk(zrpf_handle_t *handle,
				 void *chunk,
				 int len)

{

  assert(handle->init_flag == ZRPF_HANDLE_INIT_FLAG);

  MEMbufReset(handle->mbuf);
  MEMbufAdd(handle->mbuf, chunk, len);
  
  set_buffer_ptrs(handle);

  BE_to_array_32(handle->hdr, ZRPF_HDR_NBYTES_32);

  zr_params_from_BE(handle->zr_params);

  BE_to_array_32(handle->point_forecast,
		 (handle->hdr->n_forecasts *
		  sizeof(zrpf_precip_t)));

}

/****************************
 * zrpf_handle_print
 *
 * Prints out structs in handle
 */

void zrpf_handle_print(FILE *out,
		       const char *spacer,
		       zrpf_handle_t *handle)

{

  int i;
  zrpf_hdr_t *hdr = handle->hdr;

  assert(handle->init_flag == ZRPF_HANDLE_INIT_FLAG);

  fprintf(out, "\n");
  fprintf(out, "-----------------------------------\n");
  fprintf(out, "%sZR POINT FORECAST DATA\n", spacer);

  /*
   * header
   */

  fprintf(out, "%s  gauge_name: %s\n", spacer,
	  hdr->gauge_name);
  fprintf(out, "%s  gauge_lat (deg): %f\n", spacer,
	  (double) hdr->gauge_lat);
  fprintf(out, "%s  gauge_lon (deg): %f\n", spacer,
	  (double) hdr->gauge_lon);
  fprintf(out, "%s  n_forecasts: %d\n", spacer, hdr->n_forecasts);
  fprintf(out, "%s  forecast_start_time: %s\n", spacer,
	  utimstr(hdr->forecast_start_time));
  fprintf(out, "%s  forecast_delta_time (sec): %d\n", spacer,
	  hdr->forecast_delta_time);

  /*
   * zr_params
   */

  zr_params_print(out, spacer, handle->zr_params);

  /*
   * point forecast
   */

  fprintf(out, "%sPoint forecasts:\n", spacer);

  for (i = 0; i < hdr->n_forecasts; i++) {
    fprintf(out, "%s  Lead time %10d: rate (mm/hr) = %10.3f, "
	    "accum (mm) = %10.3f\n",
	    spacer, i * hdr->forecast_delta_time,
	    handle->point_forecast[i].rate,
	    handle->point_forecast[i].accum);
  }
  
  fprintf(out, "\n");
  
  fprintf(out, "%s------------------------------\n\n", spacer);
  
}

/****************************
 * set_buffer_ptrs
 *
 * Sets the pointers in the buffer
 */

static void set_buffer_ptrs(zrpf_handle_t *handle)
     
{

  ui08 *ptr;
  
  assert(handle->init_flag == ZRPF_HANDLE_INIT_FLAG);
  
  ptr = (ui08 *) MEMbufPtr(handle->mbuf);
  handle->hdr = (zrpf_hdr_t *) ptr;
  ptr += sizeof(zrpf_hdr_t);
  handle->zr_params = (zr_params_t *) ptr;
  ptr += sizeof(zr_params_t);
  handle->point_forecast = (zrpf_precip_t *) ptr;

}


