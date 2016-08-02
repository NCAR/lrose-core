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
/**********************************************************************
 * zrpf.h
 *
 * ZR point forecast data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Nov 1997
 *
 **********************************************************************/

#ifndef zrpf_h
#define zrpf_h

#ifdef __cplusplus
extern "C" {
#endif

#include <rapformats/zr.h>
#include <toolsa/membuf.h>

#define ZRPF_HANDLE_INIT_FLAG 887654321
#define ZRPF_HDR_NBYTES_32 32
#define ZRPF_HDR_GAUGE_NAME_LEN 16

/*
 * ZR Point forecast struct
 */

typedef struct {

  fl32 gauge_lat;
  fl32 gauge_lon;
  si32 n_forecasts; /* number of forecasts - includes lead time 0 */
  si32 forecast_start_time; /* UNix time */
  si32 forecast_delta_time; /* secs */
  si32 spare[3];
  char gauge_name[ZRPF_HDR_GAUGE_NAME_LEN];
  
} zrpf_hdr_t;

/*
 * point forecast precip struct
 */

typedef struct {

  fl32 rate;   /* rate in mm/hr */
  fl32 accum;  /* accumulation from chunk time, in mm */

} zrpf_precip_t;

/***********************************************************
 * zrpf_handle_t
 *
 * This is a convenience struct for dealing with a zr_params struct
 * and the associated dbz array.
 */

typedef struct {

  int init_flag;

  /* pointer to zrpf header in buffer */

  zrpf_hdr_t *hdr;

  /* pointer to zr params struct in buffer */

  zr_params_t *zr_params;

  /* pointer to forecast precip array in buffer */  

  zrpf_precip_t *point_forecast;
  
  MEMbuf *mbuf; /* buffer */

} zrpf_handle_t;

/************
 * prototypes
 */

/****************************
 * zrpf_handle_init
 *
 * Initializes handle
 */

extern void zrpf_handle_init(zrpf_handle_t *zrpf_handle);

/****************************
 * zrpf_handle_free
 *
 * Frees handle
 */

extern void zrpf_handle_free(zrpf_handle_t *zrpf_handle);

/****************************
 * zrpf_handle_alloc
 *
 * Allocates buffer space
 */

extern void zrpf_handle_alloc(zrpf_handle_t *zrpf_handle,
			      int n_forecasts);

/****************************
 * zrpf_handle_to_BE
 *
 * Swaps in place
 */

extern void zrpf_handle_to_BE(zrpf_handle_t *zrpf_handle);

/****************************
 * zrpf_handle_load_from_chunk
 *
 * 1. Loads up handle buffer from chunk.
 * 2. Sets pointers.
 * 3. Swaps from BE.
 */

extern void zrpf_handle_load_from_chunk(zrpf_handle_t *zrpf_handle,
					void *chunk, int len);

/****************************
 * zrpf_handle_print
 *
 * Prints out struct
 */

extern void zrpf_handle_print(FILE *out,
			      const char *spacer,
			      zrpf_handle_t *zrpf_handle);


#ifdef __cplusplus
}
#endif

#endif

