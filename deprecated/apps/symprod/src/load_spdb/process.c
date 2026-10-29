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
 * process.c
 *
 * Process the input file
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1996
 *
 ****************************************************************************/

#include "load_spdb.h"
#include <toolsa/utim.h>
#include <symprod/spdb_client.h>
#include <dataport/bigend.h>

static int load_chunk(si32 *valid_time, si32 *expire_time,
		      char *chunk, char *line);

static int load_times(si32 *valid_time, si32 *expire_time,
		      char *line);

void process(int chunk_len)

{

  char line[BUFSIZ];
  char *chunk;
  int line_no = 0;
  int retval;
  si32 valid_time, expire_time;
  FILE *product_file;
#ifdef NOTNOW
  spdb_handle_t db;
#endif
  spdb_chunk_ref_t chunk_ref;

  /*
   * alloc space for chunk
   */

  chunk = umalloc(chunk_len);

#ifdef NOTNOW
  /*
   * create data-base object
   */

  if (SPDB_init(&db,
		Glob->params.product_label,
		Glob->params.product_id,
		Glob->params.data_base_dir)) {
    fprintf(stderr, "ERROR - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Cannot create data base handle.\n");
    fprintf(stderr, "product_label: %s\n", Glob->params.product_label); 
    fprintf(stderr, "product_id: %ld\n", Glob->params.product_id); 
    fprintf(stderr, "data_base_dir: %s\n", Glob->params.data_base_dir); 
    tidy_and_exit(-1);
  }
#endif
  
  /*
   * open products file
   */
  
  if ((product_file = fopen(Glob->params.product_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Cannot open the product file for reading\n");
    perror(Glob->params.product_file_path);
    tidy_and_exit(-1);
  }
  
  while (!feof(product_file)) {

    /*
     * read in a line
     */
    
    if (fgets(line, BUFSIZ, product_file) != NULL) {

      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "%s", line);
      }
      line_no++;
      
      /*
       * load up a chunk
       */
      
      if (load_chunk(&valid_time, &expire_time, chunk, line) == 0) {

	chunk_ref.valid_time = valid_time;
	chunk_ref.expire_time = expire_time;
	chunk_ref.data_type = 0;
	chunk_ref.offset = 0;
	chunk_ref.len = chunk_len;

	switch (Glob->params.load_mode) {

	case LOAD_ONCE:
	  retval = SPDB_put(Glob->params.data_base_dir,
			    Glob->params.product_id,
			    Glob->params.product_label,
			    1, &chunk_ref,
			    chunk, chunk_len);
#ifdef NOTNOW
	  retval = SPDB_store(&db, 0, valid_time, expire_time,
			      chunk, chunk_len);
#endif
	  break;
	  
	case LOAD_OVERWRITE:
	  retval = SPDB_put_over(Glob->params.data_base_dir,
				 Glob->params.product_id,
				 Glob->params.product_label,
				 1, &chunk_ref,
				 chunk, chunk_len);
#ifdef NOTNOW
	  retval = SPDB_store_over(&db, 0, valid_time, expire_time,
				   chunk, chunk_len);
#endif
	  break;
	  
	case LOAD_ADD:
	  retval = SPDB_put_add(Glob->params.data_base_dir,
				Glob->params.product_id,
				Glob->params.product_label,
				1, &chunk_ref,
				chunk, chunk_len);

#ifdef NOTNOW
	  retval = SPDB_store_add(&db, 0, valid_time, expire_time,
				  chunk, chunk_len);
#endif
	  break;
	  
	} /* switch */
	
	
	if (retval) {
	  fprintf(stderr, "ERROR %s:process\n", Glob->prog_name);
	  fprintf(stderr, "Cannot store chunk\n");
	  break;
	}

      } else {

	fprintf(stderr, "ERROR %s:process\n", Glob->prog_name);
	fprintf(stderr, "Cannot decode line for chunk\n");
	fprintf(stderr, "Reading file %s, line_no %d",
		Glob->params.product_file_path, line_no);
	fprintf(stderr, "'%s'", line);

      }

    } /* if (fgets ... */
    
  } /* while */

#ifdef NOTNOW
  SPDB_free(&db);
#endif
  fclose(product_file);

}

/*************************************************************
 * load_chunk()
 *
 * Load up chunk and times
 *
 * returns 0 on success, -1 on error
 */

static int load_chunk(si32 *valid_time, si32 *expire_time,
		      char *chunk, char *line)

{
  
  char *token, *chunk_ptr;
  char sdata[BUFSIZ];
  int i, idata;
  float fdata;
  si32 si32data;
  fl32 fl32data;

  if (load_times(valid_time, expire_time, line)) {
    return (-1);
  }

  chunk_ptr = chunk;
  
  for (i = 0; i < Glob->params.data_format.len; i++) {
    
    token = strtok(NULL, " \t");
    if (token == NULL) {
      fprintf(stderr, "Cannot decode data field %d\n", i);
      return (-1);
    }
    
    switch(Glob->format_types[i]) {
      
    case FT_INT:
      if (sscanf(token, "%d", &idata) != 1) {
	fprintf(stderr, "Cannot decode data field %d\n", i);
	return (-1);
      }
      si32data = idata;
      memcpy((void *) chunk_ptr, (void *) &si32data,
	     sizeof(si32));
      BE_from_array_32((void *) chunk_ptr, sizeof(si32));
      chunk_ptr += sizeof(si32);
      break;
      
    case FT_FLOAT:
      if (sscanf(token, "%g", &fdata) != 1) {
	fprintf(stderr, "Cannot decode data field %d\n", i);
	return (-1);
      }
      fl32data = fdata;
      memcpy((void *) chunk_ptr, (void *) &fl32data,
	     sizeof(fl32));
      BE_from_array_32((void *) chunk_ptr, sizeof(fl32));
      chunk_ptr += sizeof(fl32);
      break;
      
    case FT_STRING:
      if (sscanf(token, "%s", sdata) != 1) {
	fprintf(stderr, "Cannot decode data field %d\n", i);
	return (-1);
      }
      memcpy((void *) chunk_ptr, (void *) sdata,
	     Glob->params.string_len);
      chunk_ptr += Glob->params.string_len;
      break;

    } /* switch */
    
  } /* i */

  return (0);

}

/**************************************************************
 * load_times()
 *
 * Reads either unix_time or (year month day hour min sec) from 
 * the start of the line.
 *
 * Sets valid_time and expire_time.
 *
 * returns 0 on success, -1 on failure.
 */

static int load_times(si32 *valid_time, si32 *expire_time,
		      char *line)

{

  char *token;
  UTIMstruct ut;

  token = strtok(line, " \t");

  if (Glob->params.date_time_format == UNIX_TIME) {

    if (token == NULL || sscanf(token, "%ld", &ut.unix_time) != 1) {
      fprintf(stderr, "Cannot decode unix time field\n");
      return (-1);
    }

    *valid_time = ut.unix_time;
    *expire_time = *valid_time + Glob->params.product_lifetime;

    return (0);

  } else {

    if (token == NULL || sscanf(token, "%ld", &ut.year) != 1) {
      fprintf(stderr, "Cannot decode year field\n");
      return (-1);
    }

    token = strtok(NULL, " \t");

    if (token == NULL || sscanf(token, "%ld", &ut.month) != 1) {
      fprintf(stderr, "Cannot decode month field\n");
      return (-1);
    }

    token = strtok(NULL, " \t");

    if (token == NULL || sscanf(token, "%ld", &ut.day) != 1) {
      fprintf(stderr, "Cannot decode day field\n");
      return (-1);
    }

    token = strtok(NULL, " \t");

    if (token == NULL || sscanf(token, "%ld", &ut.hour) != 1) {
      fprintf(stderr, "Cannot decode hour field\n");
      return (-1);
    }

    token = strtok(NULL, " \t");

    if (token == NULL || sscanf(token, "%ld", &ut.min) != 1) {
      fprintf(stderr, "Cannot decode min field\n");
      return (-1);
    }

    token = strtok(NULL, " \t");

    if (token == NULL || sscanf(token, "%ld", &ut.sec) != 1) {
      fprintf(stderr, "Cannot decode sec field\n");
      return (-1);
    }

    UTIMdate_to_unix(&ut);

    *valid_time = ut.unix_time;
    *expire_time = *valid_time + Glob->params.product_lifetime;

    return (0);
    
  } /* if (Glob->params.date_time_format == UNIX_TIME) */

}

