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
 * trec_gauge.c
 *
 * Routines for handling the trec gauge data
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * Oct 1997
 *
 *********************************************************************/

#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include <rapformats/trec_gauge.h>
#include <assert.h>

/****************************
 * trec_gauge_init
 *
 * Initializes handle
 */

void trec_gauge_init(trec_gauge_handle_t *tgauge)

{

  if (tgauge->init_flag != TREC_GAUGE_INIT_FLAG) {
    tgauge->mbuf = MEMbufCreate();
    tgauge->init_flag = TREC_GAUGE_INIT_FLAG;
  }

}

/****************************
 * trec_gauge_free
 *
 * Frees handle
 */

void trec_gauge_free(trec_gauge_handle_t *tgauge)

{

  MEMbufDelete(tgauge->mbuf);
  tgauge->init_flag = 0;

}

/****************************
 * trec_gauge_alloc
 *
 * Allocates buffer space
 */

void trec_gauge_alloc(trec_gauge_handle_t *tgauge,
		      int n_forecasts)
     
{

  int nbytes_needed;

  assert(tgauge->init_flag == TREC_GAUGE_INIT_FLAG);

  nbytes_needed =
    sizeof(trec_gauge_hdr_t) + n_forecasts * sizeof(fl32);

  MEMbufPrepare(tgauge->mbuf, nbytes_needed);

  tgauge->hdr = (trec_gauge_hdr_t *) MEMbufPtr(tgauge->mbuf);
  tgauge->dbz = (fl32 *) ((char *) MEMbufPtr(tgauge->mbuf) +
			  sizeof(trec_gauge_hdr_t));

}

/****************************
 * trec_gauge_to_BE
 *
 * Swaps in place
 */

void trec_gauge_to_BE(trec_gauge_handle_t *tgauge)

{

  assert(tgauge->init_flag == TREC_GAUGE_INIT_FLAG);

  BE_from_array_32(tgauge->dbz, tgauge->hdr->n_forecasts * sizeof(fl32));
  BE_from_array_32(tgauge->hdr, TREC_GAUGE_HDR_NBYTES_32);
  
}

/****************************
 * trec_gauge_load_from_chunk
 *
 * 1. Loads up handle buffer from chunk.
 * 2. Sets pointers.
 * 3. Swaps from BE.
 */

void trec_gauge_load_from_chunk(trec_gauge_handle_t *tgauge,
				void *chunk,
				int len)

{

  assert(tgauge->init_flag == TREC_GAUGE_INIT_FLAG);

  MEMbufReset(tgauge->mbuf);
  MEMbufAdd(tgauge->mbuf, chunk, len);

  tgauge->hdr = (trec_gauge_hdr_t *) MEMbufPtr(tgauge->mbuf);
  BE_to_array_32(tgauge->hdr, TREC_GAUGE_HDR_NBYTES_32);

  tgauge->dbz = (fl32 *) ((char *) MEMbufPtr(tgauge->mbuf) +
			  sizeof(trec_gauge_hdr_t));

  BE_to_array_32(tgauge->dbz, tgauge->hdr->n_forecasts * sizeof(fl32));
  
}

/****************************
 * trec_gauge_print
 *
 * Prints out struct
 */

void trec_gauge_print(FILE *out,
		      const char *spacer,
		      trec_gauge_handle_t *tgauge)

{

  int i;
  trec_gauge_hdr_t *hdr = tgauge->hdr;
  
  assert(tgauge->init_flag == TREC_GAUGE_INIT_FLAG);

  fprintf(out, "%sTREC GAUGE DATA\n", spacer);
  
  fprintf(out, "%s  Gauge name: %s\n", spacer, hdr->name);
  fprintf(out, "%s  Trec time: %s\n", spacer, utimstr(hdr->trec_time));
  fprintf(out, "%s  n_forecasts: %d\n", spacer, hdr->n_forecasts);

  fprintf(out, "%s  lat: %g\n", spacer, hdr->lat);
  fprintf(out, "%s  lon: %g\n", spacer, hdr->lon);
  fprintf(out, "%s  u: %g\n", spacer, hdr->u);
  fprintf(out, "%s  v: %g\n", spacer, hdr->v);
  fprintf(out, "%s  u_surface: %g\n", spacer, hdr->u_surface);
  fprintf(out, "%s  v_surface: %g\n", spacer, hdr->v_surface);
  fprintf(out, "%s  wt_surface: %g\n", spacer, hdr->wt_surface);
  fprintf(out, "%s  forecast_delta_time: %g\n", spacer,
	  hdr->forecast_delta_time);

  fprintf(out, "%s  dbz at forecast times:\n", spacer);

  for (i = 0; i < hdr->n_forecasts; i++) {
    fprintf(out, "%s  Time %d: dbz = %g\n", spacer, i, tgauge->dbz[i]);
  }

  fprintf(out, "\n\n");
  
}

