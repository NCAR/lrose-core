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
/*****************************************************************
 * PRINT_CHUNKS.H
 */

#include <Spdb/sounding.h>

#include <rapformats/bdry.h> 
#include <rapformats/flt_path.h>
#include <rapformats/fos.h>
#include <rapformats/kavltg.h>
#include <rapformats/station_reports.h>
#include <rapformats/trec_gauge.h>
#include <rapformats/zr.h>
#include <rapformats/zrpf.h>
#include <toolsa/udatetime.h>
#include <rapformats/station_reports.h>
#include <rapformats/pirep.h>
#include <titan/ac_posn.h>
#include <rapformats/tstorm_spdb.h>

#include <symprod/spdb_products.h>
#include <symprod/spdb_client.h>
#include <symprod/symprod.h>

void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *chunk_hdr);

void print_ac_posn_data(FILE *stream,
			       spdb_chunk_ref_t *chunk_hdr,
			       void *chunk_data);

void print_bdry_data(FILE *stream,
			    spdb_chunk_ref_t *chunk_hdr,
			    void *chunk_data);

void print_flt_path_data(FILE *stream,
				spdb_chunk_ref_t *chunk_hdr,
				void *chunk_data);

void print_history_forecast_data(FILE *stream,
					spdb_chunk_ref_t *chunk_hdr,
					void *chunk_data);

void print_kav_ltg_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data);

void print_sigmet_data(FILE *stream,
			      spdb_chunk_ref_t *hdr,
			      void *data);

void print_stn_report(FILE *stream,
			     spdb_chunk_ref_t *chunk_hdr,
			     void *chunk_data);

void print_symprod_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data);

void print_tstorms_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data);

void print_trec_gauge_data(FILE *stream,
				  spdb_chunk_ref_t *hdr,
				  void *data);

void print_zr_params_data(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data);

void print_zrpf_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data);

void print_sounding_data(FILE *stream,
               spdb_chunk_ref_t *hdr,
		void *data);

void print_pirep_data(FILE *stream,
               spdb_chunk_ref_t *hdr,
		void *data);

void print_ascii_data(FILE *stream,
               spdb_chunk_ref_t *hdr,
		void *data);
