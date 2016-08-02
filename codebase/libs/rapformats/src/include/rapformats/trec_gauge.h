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
/********************************************************
 * TREC_GAUGE.H
 *
 * Support for trec-gauge data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307
 * October 1997
 */

#ifndef trec_gauge_h
#define trec_gauge_h

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>
#include <toolsa/membuf.h>

#define TREC_GAUGE_INIT_FLAG 987654321
#define TREC_GAUGE_NAME_LEN 16
#define TREC_GAUGE_HDR_NBYTES_32 40

/***********************************************************
 * trec_gauge_t
 *
 * This struct holds data, for gauge locations, sampled from a
 * trec output file. The intention is to use this data for
 * radar gauge comparison.
 *
 * The struct is followed by an array of n_forecast fl32's,
 * which hold the reflectivity at each point upstream of the
 * gauge.
 */

typedef struct {

  fl32 lat;
  fl32 lon;
  fl32 u;
  fl32 v;
  fl32 forecast_delta_time;
  si32 n_forecasts;
  si32 trec_time;
  fl32 u_surface;
  fl32 v_surface;
  fl32 wt_surface;
  char name[TREC_GAUGE_NAME_LEN];

} trec_gauge_hdr_t;

/***********************************************************
 * trec_gauge_handle_t
 *
 * This is a convenience struct for dealing with a trec_gauge struct
 * and the associated dbz array.
 */

typedef struct {

  int init_flag;

  trec_gauge_hdr_t *hdr; /* pointer to header in buffer */

  fl32 *dbz; /* pointer to dbz array in buffer */

  MEMbuf *mbuf; /* buffer */

} trec_gauge_handle_t;

/*
 * prototypes
 */

/****************************
 * trec_gauge_init
 *
 * Initializes handle
 */

extern void trec_gauge_init(trec_gauge_handle_t *tgauge);

/****************************
 * trec_gauge_free
 *
 * Frees handle
 */

extern void trec_gauge_free(trec_gauge_handle_t *tgauge);

/****************************
 * trec_gauge_alloc
 *
 * Allocates buffer space
 */

extern void trec_gauge_alloc(trec_gauge_handle_t *tgauge,
			     int n_forecasts);

/****************************
 * trec_gauge_to_BE
 *
 * Swaps in place
 */

extern void trec_gauge_to_BE(trec_gauge_handle_t *tgauge);

/****************************
 * trec_gauge_load_from_chunk
 *
 * 1. Loads up handle buffer from chunk.
 * 2. Sets pointers.
 * 3. Swaps from BE.
 */

extern void trec_gauge_load_from_chunk(trec_gauge_handle_t *tgauge,
				       void *chunk, int len);

/****************************
 * trec_gauge_print
 *
 * Prints out struct
 */

extern void trec_gauge_print(FILE *out,
			     const char *spacer,
			     trec_gauge_handle_t *tgauge);


#ifdef __cplusplus
}
#endif

#endif
