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
/************************************************************************
 * spdb_query.h : header file for spdb_query program
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1998
 *
 * Mike Dixon
 *
 ************************************************************************/

#ifndef spdb_query_H
#define spdb_query_H

#include <toolsa/os_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/param.h>

#ifdef SUNOS4
extern int tolower(int c);
#endif

#include <symprod/spdb.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>

/*
 * function prototypes
 */

extern void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *chunk_hdr);

extern void print_ascii_data(FILE *stream,
			     spdb_chunk_ref_t *hdr,
			     void *data);

extern void print_ac_posn_data(FILE *stream,
			       spdb_chunk_ref_t *chunk_hdr,
			       void *chunk_data);

extern void print_ac_posn_wmod_data(FILE *stream,
				    spdb_chunk_ref_t *hdr,
				    void *data);

extern void print_ac_data_data(FILE *stream,
			       spdb_chunk_ref_t *chunk_hdr,
			       void *chunk_data);

extern void print_bdry_data(FILE *stream,
			    void *chunk_data);

extern void print_flt_path_data(FILE *stream,
				void *chunk_data);

extern void print_flt_route_data(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data);

extern void print_GenPt_data(FILE *stream,
			     spdb_chunk_ref_t *hdr,
			     void *data);

extern void print_GenPoly_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data);

extern void print_HydroStation_data(FILE *stream,
				    spdb_chunk_ref_t *hdr,
				    void *data);

extern void print_history_forecast_data(FILE *stream,
					spdb_chunk_ref_t *chunk_hdr,
					void *chunk_data);

extern void print_irsa_forecast_data(FILE *stream,
				     void *chunk_data);

extern void print_ltg_data(FILE *stream,
			   spdb_chunk_ref_t *hdr,
			   void *data);

extern void print_pirep_data(FILE *stream,
			     spdb_chunk_ref_t *chunk_hdr,
			     void *chunk_data);

extern void print_posn_rpt_data(FILE *stream,
				spdb_chunk_ref_t *hdr,
				void *data);

extern void print_sigmet_data(FILE *stream,
			      void *data);

extern void print_sndg_data(FILE *stream,
			    void *chunk_data);

extern void print_stn_report(FILE *stream,
			     void *chunk_data);

extern void print_symprod_data(FILE *stream,
			       void *data);

extern void print_tstorms_data(FILE *stream,
			       void *data);

extern void print_trec_gauge_data(FILE *stream,
				  spdb_chunk_ref_t *hdr,
				  void *data);

extern void print_vergrid_region(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data);

extern void print_wx_hazard_data(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data);

extern void print_zr_params_data(FILE *stream,
				 void *data);

extern void print_zrpf_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data);


#endif
