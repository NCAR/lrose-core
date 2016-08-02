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
/***************************************************************************
 * so_print.c
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1996
 *
 ****************************************************************************/

#include "print_spdb.h"
#include <symprod/spdb.h>
#include <dataport/bigend.h>

static void print_chunk(FILE *out, char *chunk);

void do_print(int chunk_len)

{

  int i;
  si32 n_chunks;
  char *chunks;
  spdb_chunk_ref_t *refs;
  spdb_handle_t db;
  
  /*
   * create data-base object
   */
  
  if (SPDB_init(&db, NULL, -1,
		Glob->params.data_base_dir)) {
    fprintf(stderr, "ERROR - %s:do_print\n", Glob->prog_name);
    fprintf(stderr, "Cannot create data base handle.\n");
    fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
    tidy_and_exit(-1);
  }
  
  if (Glob->hdr_time >= 0) {

    if (SPDB_print_header(&db, Glob->hdr_time, stdout)) {
      fprintf(stderr, "ERROR - %s:do_print\n", Glob->prog_name);
      fprintf(stderr, "Cannot print header.\n");
      fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
      tidy_and_exit(-1);
    }
    fflush(stdout);

  } else {

    if (Glob->closest_time >= 0) {

      if (SPDB_fetch_closest(&db, 0,
			     Glob->closest_time,
			     Glob->params.time_margin,
			     &n_chunks,
			     &refs, (void **) &chunks)) {
	fprintf(stderr, "ERROR - %s:do_print\n", Glob->prog_name);
	fprintf(stderr, "Cannot perform SPDB_fetch_closest.\n");
	fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
	tidy_and_exit(-1);
      }
      
    } else if (Glob->valid_time >= 0) {

      if (SPDB_fetch_valid(&db, 0,
			   Glob->valid_time,
			   &n_chunks,
			   &refs, (void **) &chunks)) {
	fprintf(stderr, "ERROR - %s:do_print\n", Glob->prog_name);
	fprintf(stderr, "Cannot perform SPDB_fetch_valid.\n");
	fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
	tidy_and_exit(-1);
      }
      
    } else {
      
      if (SPDB_fetch_interval(&db, 0,
			      Glob->start_time,
			      Glob->end_time,
			      &n_chunks,
			      &refs, (void **) &chunks)) {
	fprintf(stderr, "ERROR - %s:do_print\n", Glob->prog_name);
	fprintf(stderr, "Cannot perform SPDB_fetch_interval.\n");
	fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
	tidy_and_exit(-1);
      }
      
    }
    
    for (i = 0; i < n_chunks; i++) {
      if (Glob->params.full_printout) {
	fprintf(stdout, "Chunk %4d: valid_time %s expire_time %s len %5d\n",
		i,
		UTIMstr(refs[i].valid_time), UTIMstr(refs[i].expire_time),
		refs[i].len);
      } else {
	fprintf(stdout, "%s :",	UTIMstr(refs[i].valid_time));
      }
      if (Glob->params.decode_chunks) {
	if (refs[i].len == chunk_len) {
	  print_chunk(stdout, chunks + refs[i].offset);
	} else {
	  fprintf(stderr, "Chunk len incorrect, should be %d\n",
		  chunk_len);
	}
      }
      fflush(stdout);
    } /* i */

  } /* if (Glob->hdr_time >= 0) */

  SPDB_free(&db);
  
}

/*************************************************************
 * print_chunk()
 *
 * Prints chunk data
 *
 */

static void print_chunk(FILE *out, char *chunk)

{
  
  char *data_ptr;
  int i;
  si32 *si32data;
  fl32 *fl32data;

  data_ptr = chunk;
  
  for (i = 0; i < Glob->params.data_format.len; i++) {
    
    switch(Glob->format_types[i]) {
      
    case FT_INT:
      si32data = (si32 *) data_ptr;
      BE_to_array_32((void *) si32data, sizeof(si32));
      fprintf(out, "%d", *si32data);
      data_ptr += sizeof(si32);
      break;
      
    case FT_FLOAT:
      fl32data = (fl32 *) data_ptr;
      BE_to_array_32((void *) fl32data, sizeof(fl32));
      fprintf(out, "%g", *fl32data);
      data_ptr += sizeof(fl32);
      break;
      
    case FT_STRING:
      fprintf(out, "%s", data_ptr);
      data_ptr += Glob->params.string_len;
      break;
      
    } /* switch */

    if (i < Glob->params.data_format.len - 1) {
      fprintf(out, " ");
    } else {
      fprintf(out, "\n");
    }
    
  } /* i */
  
}

