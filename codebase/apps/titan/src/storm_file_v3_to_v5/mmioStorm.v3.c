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
/*************************************************************************
 *
 * mmioStorm.v3.c
 *
 * Memory-mapped IO version of RfStorm.c routines. These routines
 * are intended for read-only purposes, and will only work on a machine
 * which does not require byte-swapping relative to XDR.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * November 1994
 *
 **************************************************************************/

#include "storm_file_v3_to_v5.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>

#define MAX_SEQ 256

/*************************************************************************
 *
 * mmiov3CloseStormData()
 *
 * Closes the storm data mmio buffer
 *
 **************************************************************************/

void mmiov3CloseStormData(storm_v3_file_index_t *s_handle,
			  char *calling_routine)
     
{

  if (s_handle->data_mmio_buf != NULL) {

    if (munmap((caddr_t) s_handle->data_mmio_buf,
	       s_handle->data_mmio_len)) {
      fprintf(stderr, "WARNING - %s:mmiov3CloseStormData\n", calling_routine);
      fprintf(stderr, "Cannot unmap memory for %s\n",
	      s_handle->data_file_path);
    }
    
    s_handle->data_mmio_buf = (char *) NULL;
    s_handle->data_mmio_len = 0;

  }

  if (s_handle->data_file_path != NULL) {
    ufree ((char *) s_handle->data_file_path);
    s_handle->data_file_path = (char *) NULL;
  }

}

/*************************************************************************
 *
 * mmiov3CloseStormFiles()
 *
 * Closes the storm header and data files
 *
 **************************************************************************/

int mmiov3CloseStormFiles(storm_v3_file_index_t *s_handle,
			  char *calling_routine)
     
{
 
  mmiov3CloseStormHeader(s_handle, calling_routine);
  mmiov3CloseStormData(s_handle, calling_routine);

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * mmiov3CloseStormHeader()
 *
 * Closes the storm header mmio buffer
 *
 **************************************************************************/

void mmiov3CloseStormHeader(storm_v3_file_index_t *s_handle,
			    char *calling_routine)
     
{

  if (s_handle->header_mmio_buf != NULL) {

    if (munmap((caddr_t) s_handle->header_mmio_buf,
	       s_handle->header_mmio_len)) {
      fprintf(stderr, "WARNING - %s:mmiov3CloseStormHeader\n", calling_routine);
      fprintf(stderr, "Cannot unmap memory for %s\n",
	      s_handle->header_file_path);
    }
    
    s_handle->header_mmio_buf = (char *) NULL;
    s_handle->header_mmio_len = 0;

  }

  if (s_handle->header_file_path != NULL) {
    ufree ((char *) s_handle->header_file_path);
    s_handle->header_file_path = (char *) NULL;
  }

}

/*************************************************************************
 *
 * mmiov3OpenStormFiles()
 *
 * Opens the storm header and data files
 *
 * The storm header file path must have been set
 *
 **************************************************************************/

#define THIS_ROUTINE "mmiov3OpenStormFiles"

int mmiov3OpenStormFiles(storm_v3_file_index_t *s_handle,
			 char *mode,
			 char *header_file_path,
			 char *data_file_ext,
			 char *calling_routine)
     
{
 
  char calling_sequence[MAX_SEQ];
  char tmp_path[MAX_PATH_LEN];
  char data_file_path[MAX_PATH_LEN];
  char *chptr;
  int header_file_len = 0;
  int data_file_len = 0;
  int header_fd, data_fd;
  struct stat sbuf;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * check the mode
   */

  if (strcmp(mode, "r")) {
    fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
    fprintf(stderr,
	    "Mode %s not acceptable - mmio routines are read-only\n", mode);
    fprintf(stderr, "Do not use memory-mapped io for this application\n");
    return (R_FAILURE);
  }

  /*
   * header file
   * ===========
   */
  
  /*
   * get header file stats
   */

  Rf_file_uncompress(header_file_path);

  if (stat(header_file_path, &sbuf) < 0) {
    fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
    fprintf(stderr,
	    "Cannot stat header file %s\n", header_file_path);
    return (R_FAILURE);
  }
  header_file_len = sbuf.st_size;
    
  /*
   * check header file name and len
   */
  
  if ((s_handle->header_file_path == NULL) ||
      strcmp(s_handle->header_file_path, header_file_path) ||
      (header_file_len != s_handle->header_mmio_len)) {
    
    /*
     * file differs from previous time, so close
     */
    
    mmiov3CloseStormHeader(s_handle, calling_sequence);
    
    /*
     * open header as required
     */
    
    if ((header_fd = open(header_file_path, O_RDONLY)) < 0) {
      fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
      fprintf(stderr,
	      "Cannot open header file %s\n", header_file_path);
      return (R_FAILURE);
    }
    
    if ((s_handle->header_mmio_buf = mmap(0, header_file_len,
					PROT_READ,
					MAP_SHARED,
					header_fd, 0)) == (caddr_t) -1) {
      fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
      fprintf(stderr,
	      "Cannot map header file %s\n", header_file_path);
      close(header_fd);
      s_handle->header_mmio_buf = (char *) NULL;
      s_handle->header_mmio_len = 0;
      return (R_FAILURE);
    }
  
    /*
     * close the file descriptor - this does not unmap the file
     */

    close(header_fd);

    /*
     * set the mapped length
     */

    s_handle->header_mmio_len = header_file_len;

  } /* if (strcmp(s_handle->header_file_path, header_file_path) ... */

  /*
   * read the header
   */
  
  if (mmiov3ReadStormHeader(s_handle, calling_sequence))
    return (R_FAILURE);
  
  /*
   * compute the file path from the header file path and
   * the data file name
   */
  
  strncpy(tmp_path, header_file_path, MAX_PATH_LEN);
  
  /*
   * if dir path has slash, get pointer to that and end the sting
   * immediately after
   */
  
  if ((chptr = strrchr(tmp_path, '/')) != NULL) {
    *(chptr + 1) = '\0';
    sprintf(data_file_path, "%s%s", tmp_path,
	    s_handle->header->data_file_name);
  } else {
    strcpy(data_file_path, s_handle->header->data_file_name);
  }
  
  /*
   * set header file path in index
   */
  
  if (s_handle->header_file_path != NULL)
    ufree ((char *) s_handle->header_file_path);

  s_handle->header_file_path = (char *) umalloc
    ((ui32) (strlen(header_file_path)+ 1));

  strcpy(s_handle->header_file_path, header_file_path);

  /*
   * data file
   * =========
   */
  
  /*
   * get data file stats
   */

  Rf_file_uncompress(data_file_path);

  if (stat(data_file_path, &sbuf) < 0) {
    fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
    fprintf(stderr,
	    "Cannot stat data file %s\n", data_file_path);
    return (R_FAILURE);
  }
  data_file_len = sbuf.st_size;
    
  /*
   * check data file name and len
   */
  
  if ((s_handle->data_file_path == NULL) ||
      strcmp(s_handle->data_file_path, data_file_path) ||
      (data_file_len != s_handle->data_mmio_len)) {
    
    /*
     * file differs from previous time, so close
     */
    
    mmiov3CloseStormData(s_handle, calling_sequence);
    
    /*
     * open data as required
     */
    
    if ((data_fd = open(data_file_path, O_RDONLY)) < 0) {
      fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
      fprintf(stderr,
	      "Cannot open data file %s\n", data_file_path);
      return (R_FAILURE);
    }
    
    if ((s_handle->data_mmio_buf = mmap(0, data_file_len,
				      PROT_READ,
				      MAP_SHARED,
				      data_fd, 0)) == (caddr_t) -1) {
      fprintf(stderr, "ERROR - %s:mmiov3OpenStormFiles\n", calling_routine);
      fprintf(stderr,
	      "Cannot map data file %s\n", data_file_path);
      close(data_fd);
      s_handle->data_mmio_buf = (char *) NULL;
      s_handle->data_mmio_len = 0;
      return (R_FAILURE);
    }
  
    /*
     * close the file descriptor - this does not unmap the file
     */
    
    close(data_fd);
    
    /*
     * set the mapped length
     */

    s_handle->data_mmio_len = data_file_len;

  } /* if (strcmp(s_handle->data_file_path, data_file_path) ... */

  /*
   * set data file path in index
   */

  s_handle->data_file_label = s_handle->data_mmio_buf;

  if (s_handle->data_file_path != NULL)
    ufree ((char *) s_handle->data_file_path);

  s_handle->data_file_path = (char *) umalloc
    ((ui32) (strlen(data_file_path)+ 1));

  strcpy(s_handle->data_file_path, data_file_path);

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * mmiov3ReadStormHeader()
 *
 * reads in the storm_v3_file_header_t structure from a storm
 * properties file.
 *
 * Note - space for the header is allocated if header_allocated is FALSE.
 * If not, previous allocation is assumed.
 *
 * If the file label is passed in as NULL, the storm file label
 * from the file is stored. If the label is non-null, the two are
 * compared and an error is returned if they are different.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "mmiov3ReadStormheader"

int mmiov3ReadStormHeader(storm_v3_file_index_t *s_handle,
			  char *calling_routine)
     
{
  
  char *file_label;
  char calling_sequence[MAX_SEQ];
  si32 n_scans;
  si32 offset;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * read in file label
   */
  
  offset = 0;
  file_label = (char *) (s_handle->header_mmio_buf + offset);
  offset += R_FILE_LABEL_LEN;

  /*
   * if the label passed in is non-null, check that the label matches
   * that expected. If the label is null, store the file label there.
   */
  
  if (s_handle->header_file_label != NULL) {
    
    if (strcmp(file_label, s_handle->header_file_label)) {
      fprintf(stderr, "ERROR - %s:%s:mmiov3ReadStormHeader\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "File does not contain correct type storm properties file.\n");
      fprintf(stderr, "File label is '%s'\n", file_label);
      fprintf(stderr, "File label should be '%s'.\n",
	      s_handle->header_file_label);
      
      return (R_FAILURE);
      
    }
    
  } else {
    
    s_handle->header_file_label = (char *) ucalloc
      ((ui32) R_FILE_LABEL_LEN, (ui32) sizeof(char));
    
    memset ((void *)  s_handle->header_file_label,
            (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(s_handle->header_file_label, file_label);
    
  } /* if (s_handle->header_file_label != NULL) */
  
  /*
   * read in header
   */

  s_handle->header = (storm_v3_file_header_t *)
    (s_handle->header_mmio_buf + offset);
  offset += sizeof(storm_v3_file_header_t);
  
  /*
   * read in scan offset
   */
  
  n_scans = s_handle->header->n_scans;
  s_handle->scan_offsets = (si32 *) (s_handle->header_mmio_buf + offset);
  offset += n_scans * sizeof(si32);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * mmiov3ReadStormProps()
 *
 * reads in the storm property data for a given storm in a scan, given the
 * scan and index structures as inputs.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "mmiov3ReadStormProps"

int mmiov3ReadStormProps(storm_v3_file_index_t *s_handle,
			 si32 storm_num,
			 char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 n_layers, n_dbz_intervals, n_runs;
  si32 offset;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * store storm number
   */
  
  s_handle->storm_num = storm_num;
  
  /*
   * return early if nstorms is zero
   */
  
  if (s_handle->scan->nstorms == 0)
    return (R_SUCCESS);
  
  n_layers = s_handle->gprops[storm_num].n_layers;
  n_dbz_intervals = s_handle->gprops[storm_num].n_dbz_intervals;
  n_runs = s_handle->gprops[storm_num].n_runs;
  
  /*
   * read in layer props
   */

  offset = s_handle->gprops[storm_num].layer_props_offset;
  s_handle->layer = (storm_v3_file_layer_props_t *)
    (s_handle->data_mmio_buf + offset);
  
  /*
   * read in histogram data
   */
  
  offset = s_handle->gprops[storm_num].dbz_hist_offset;
  s_handle->hist = (storm_v3_file_dbz_hist_t *)
    (s_handle->data_mmio_buf + offset);

  /*
   * read in runs
   */

  offset = s_handle->gprops[storm_num].runs_offset;
  s_handle->runs = (storm_v3_file_run_t *)
    (s_handle->data_mmio_buf + offset);

  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * mmiov3ReadStormScan()
 *
 * reads in the scan info for a particular scan in a storm properties
 * file.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "mmiov3ReadStormScan"

int mmiov3ReadStormScan(storm_v3_file_index_t *s_handle,
			si32 scan_num,
			char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nstorms;
  si32 offset;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * read in scan struct
   */

  offset = s_handle->scan_offsets[scan_num];
  s_handle->scan = (storm_v3_file_scan_header_t *)
    (s_handle->data_mmio_buf + offset);

  /*
   * return early if nstorms is zero
   */
  
  nstorms = s_handle->scan->nstorms;
  
  if (nstorms == 0)
    return (R_SUCCESS);
  
  /*
   * read in global props
   */
  
  offset = s_handle->scan->gprops_offset;
  s_handle->gprops = (storm_v3_file_global_props_t *)
    (s_handle->data_mmio_buf + offset);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE


  
