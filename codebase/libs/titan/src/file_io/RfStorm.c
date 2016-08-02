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
 * RfStorm.c
 *
 * part of the rfutil library - radar file access
 *
 * Storm file access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * October 1991
 *
 **************************************************************************/

#include <titan/storm.h>
#include <dataport/bigend.h>
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_SEQ 256
#define N_ALLOC 5

/*************************************************************************
 *
 * RfAllocStormHeader()
 *
 * part of the rfutil library - radar file access
 *
 * allocates space for the storm_file_header_t structure
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocStormHeader(storm_file_handle_t *s_handle,
		       const char *calling_routine)
     
{

  /*
   * allocate space for s_handle->file_header
   */

  if (!s_handle->header_allocated) {

    s_handle->header = (storm_file_header_t *)
      ucalloc((ui32) 1, (ui32) sizeof(storm_file_header_t));

    s_handle->header_allocated = TRUE;

  } /* if (!s_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocStormProps()
 *
 * part of the rfutil library - radar file access
 *
 * allocates space for the layer props and dbz hist
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocStormProps(storm_file_handle_t *s_handle,
		      si32 n_layers, si32 n_dbz_intervals,
		      si32 n_runs, si32 n_proj_runs,
		      const char *calling_routine)
     
{

  /*
   * allocate or reallocate for layer props
   */

  if (!s_handle->props_allocated) {
    
    s_handle->max_layers = n_layers;

    s_handle->layer = (storm_file_layer_props_t *)
      ucalloc((ui32) n_layers, (ui32) sizeof(storm_file_layer_props_t));

  } else if (n_layers > s_handle->max_layers) {

    s_handle->max_layers = n_layers;

    s_handle->layer = (storm_file_layer_props_t *)
      urealloc((char *) s_handle->layer,
	       (ui32) n_layers * sizeof(storm_file_layer_props_t));

  }

  /*
   * allocate or reallocate array for dbz histogram entries
   */

  if (!s_handle->props_allocated) {

    s_handle->max_dbz_intervals = n_dbz_intervals;

    s_handle->hist = (storm_file_dbz_hist_t *)
      ucalloc((ui32) n_dbz_intervals, (ui32) sizeof(storm_file_dbz_hist_t));

  } else if (n_dbz_intervals > s_handle->max_dbz_intervals) {

    s_handle->max_dbz_intervals = n_dbz_intervals;

    s_handle->hist = (storm_file_dbz_hist_t *)
      urealloc((char *) s_handle->hist,
	       (ui32) n_dbz_intervals * sizeof(storm_file_dbz_hist_t));

  }

  /*
   * allocate or reallocate for runs
   */

  if (!s_handle->props_allocated) {
    
    s_handle->max_runs = n_runs;

    s_handle->runs = (storm_file_run_t *)
      ucalloc((ui32) n_runs, (ui32) sizeof(storm_file_run_t));

  } else if (n_runs > s_handle->max_runs) {

    s_handle->max_runs = n_runs;

    s_handle->runs = (storm_file_run_t *)
      urealloc((char *) s_handle->runs,
	       (ui32) n_runs * sizeof(storm_file_run_t));

  }

  /*
   * allocate or reallocate for proj_runs
   */

  if (!s_handle->props_allocated) {
    
    s_handle->max_proj_runs = n_proj_runs;

    s_handle->proj_runs = (storm_file_run_t *)
      ucalloc((ui32) n_proj_runs, (ui32) sizeof(storm_file_run_t));

  } else if (n_proj_runs > s_handle->max_proj_runs) {

    s_handle->max_proj_runs = n_proj_runs;

    s_handle->proj_runs = (storm_file_run_t *)
      urealloc((char *) s_handle->proj_runs,
	       (ui32) n_proj_runs * sizeof(storm_file_run_t));

  }

  s_handle->props_allocated = TRUE;

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocStormScan()
 *
 * part of the rfutil library - radar file access
 *
 * allocates scan struct and gprops array
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocStormScan(storm_file_handle_t *s_handle,
		     si32 nstorms, const char *calling_routine)
     
{

  /*
   * ensure mem for at least 1 storm
   */

  if (nstorms == 0)
    nstorms = 1;

  /*
   * allocate space for scan struct
   */
  
  if (!s_handle->scan_allocated) {

    s_handle->scan = (storm_file_scan_header_t *)
      ucalloc((ui32) 1, (ui32) sizeof(storm_file_scan_header_t));

    s_handle->max_storms = nstorms;

    s_handle->gprops = (storm_file_global_props_t *)
      ucalloc((ui32) nstorms, (ui32) sizeof(storm_file_global_props_t));

    s_handle->scan_allocated = TRUE;

  } else if (nstorms > s_handle->max_storms) {

    s_handle->max_storms = nstorms;

    s_handle->gprops = (storm_file_global_props_t *)
      urealloc((char *) s_handle->gprops,
	       (ui32) nstorms * sizeof(storm_file_global_props_t));

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocStormScanOffsets()
 *
 * part of the rfutil library - radar file access
 *
 * allocates space for the scan offset array.
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocStormScanOffsets(storm_file_handle_t *s_handle,
			    si32 n_scans_needed,
			    const char *calling_routine)
     
{

  si32 n_realloc;

  if (s_handle->n_scans_allocated < n_scans_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
      
    if (s_handle->n_scans_allocated == 0) {
      
      s_handle->n_scans_allocated = n_scans_needed + N_ALLOC;

      s_handle->scan_offsets = (si32 *) umalloc
	((ui32) (s_handle->n_scans_allocated * sizeof(si32)));

    } else {

      n_realloc = n_scans_needed + N_ALLOC;
      
      s_handle->scan_offsets = (si32 *) urealloc
	((char *) s_handle->scan_offsets,
	 (ui32) (n_realloc * sizeof(si32)));
      
      s_handle->n_scans_allocated = n_realloc;
      
    } /* if (s_handle->n_scans_allocated == 0) */

    if (s_handle->scan_offsets == NULL) {
      fprintf(stderr, "ERROR - %s:RfAllocStormScanOffsets\n",
	      calling_routine);
      return (R_FAILURE);
    }

  } /* if (s_handle->n_scans_allocated < n_scans_needed) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfCloseStormFiles()
 *
 * part of the rfutil library - radar file access
 *
 * Closes the storm header and data files
 *
 **************************************************************************/

int RfCloseStormFiles(storm_file_handle_t *s_handle,
		      const char *calling_routine)
     
{

  /*
   * close the header file
   */

  if (s_handle->header_file != NULL) {
    if (fclose(s_handle->header_file)) {
      fprintf(stderr, "WARNING - %s:RfCloseStormFiles\n", calling_routine);
      perror(s_handle->header_file_path);
    }
    s_handle->header_file = (FILE *) NULL;
  }

  /*
   * close the data file
   */
  
  if (s_handle->data_file != NULL) {
    if (fclose(s_handle->data_file)) {
      fprintf(stderr, "WARNING - %s:RfCloseStormFiles\n", calling_routine);
      perror(s_handle->data_file_path);
    }
    s_handle->data_file = (FILE *) NULL;
  }

  /*
   * free up resources
   */

  if (s_handle->header_file_path != NULL) {
    ufree ((char *) s_handle->header_file_path);
    s_handle->header_file_path = (char *) NULL;
  }

  if (s_handle->data_file_path != NULL) {
    ufree ((char *) s_handle->data_file_path);
    s_handle->data_file_path = (char *) NULL;
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFlushStormFiles()
 *
 * part of the rfutil library - radar file access
 *
 * Flushes the storm header and data files
 *
 **************************************************************************/

int RfFlushStormFiles(storm_file_handle_t *s_handle,
		      const char *calling_routine)
     
{
 
  /*
   * flush the header file
   */
  
  if (fflush(s_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:RfFlushStormFiles\n", calling_routine);
    fprintf(stderr, "Flushing storm header file.\n");
    perror(s_handle->header_file_path);
  }

  /*
   * flush the data file
   */
  
  if (fflush(s_handle->data_file)) {
    fprintf(stderr, "WARNING - %s:RfFlushStormFiles\n", calling_routine);
    fprintf(stderr, "Flushing storm data file.\n");
    perror(s_handle->data_file_path);
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeStormHeader()
 *
 * part of the rfutil library - radar file access
 *
 * frees the storm_file_header_t structure and offset array
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeStormHeader(storm_file_handle_t *s_handle,
		      const char *calling_routine)
     
{

  if (s_handle->header_allocated) {
    
    ufree(s_handle->header);
    s_handle->header_allocated = FALSE;

  } /* if (s_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeStormProps()
 *
 * part of the rfutil library - radar file access
 *
 * frees the storm property data arrays
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeStormProps(storm_file_handle_t *s_handle,
		     const char *calling_routine)
     
{
  
  if (s_handle->props_allocated) {
      
    ufree ((char *) s_handle->layer);
    ufree ((char *) s_handle->hist);
    ufree ((char *) s_handle->runs);
    ufree ((char *) s_handle->proj_runs);
    s_handle->max_layers = 0;
    s_handle->max_dbz_intervals = 0;
    s_handle->max_runs = 0;
    s_handle->max_proj_runs = 0;
    s_handle->props_allocated = FALSE;

  } /* if (s_handle->props_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeStormScan()
 *
 * part of the rfutil library - radar file access
 *
 * frees the scan structures
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeStormScan(storm_file_handle_t *s_handle,
		    const char *calling_routine)
     
{

  if (s_handle->scan_allocated) {

    ufree ((char *) s_handle->scan);
    ufree ((char *) s_handle->gprops);
    s_handle->max_storms = 0;
    s_handle->scan_allocated = FALSE;

  } /* if (s_handle->scan_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeStormScanOffsets()
 *
 * part of the rfutil library - radar file access
 *
 * frees the scan offset array
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeStormScanOffsets(storm_file_handle_t *s_handle,
			   const char *calling_routine)
     
{

  if (s_handle->n_scans_allocated) {
    
    ufree(s_handle->scan_offsets);
    s_handle->n_scans_allocated = 0;

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfOpenStormFiles()
 *
 * part of the rfutil library - radar file access
 *
 * Opens the storm header and data files
 *
 * The storm header file path must have been set
 *
 **************************************************************************/

#define THIS_ROUTINE "RfOpenStormFiles"

int RfOpenStormFiles(storm_file_handle_t *s_handle,
		     const char *mode,
		     char *header_file_path,
		     const char *data_file_ext,
		     const char *calling_routine)
     
{
 
  char file_label[R_FILE_LABEL_LEN];
  char calling_sequence[MAX_SEQ];
  char tmp_path[MAX_PATH_LEN];
  char data_file_path[MAX_PATH_LEN];
  char *chptr;

  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * close files
   */

  RfCloseStormFiles(s_handle, calling_sequence);
  
  /*
   * open the header file
   */
  
  if ((s_handle->header_file =
       Rf_fopen_uncompress(header_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:RfOpenStormFiles\n", calling_routine);
    fprintf(stderr, "Cannot open storm header file '%s'\n",
	    header_file_path);
    return (R_FAILURE);
  }

  /*
   * compute the data file name
   */
   
  if (*mode == 'r') {

    /*
     * read the header if the file is opened for reading
     */
   
    if (RfReadStormHeader(s_handle, calling_sequence))
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

  } else {

    /*
     * file opened for writing, use ext to compute file name
     */

    if (data_file_ext == NULL) {
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr,
	      "Must provide data file extension for file creation\n");
      return (R_FAILURE);
    }

    strncpy(tmp_path, header_file_path, MAX_PATH_LEN);
    
    if ((chptr = strrchr(tmp_path, '.')) == NULL) {
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr, "Header file must have extension : %s\n",
	      header_file_path);
      return (R_FAILURE);
    }

    *(chptr + 1) = '\0';
    sprintf(data_file_path, "%s%s", tmp_path, data_file_ext);

  } /* if (*mode == 'r') */
    
  /*
   * open the data file
   */
  
  if ((s_handle->data_file =
       Rf_fopen_uncompress(data_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:RfOpenStormFiles\n", calling_routine);
    fprintf(stderr, "Cannot open storm data file '%s'\n",
	    data_file_path);
    return (R_FAILURE);
  }

  /*
   * In write mode, write file labels
   */
   
  if (*mode == 'w') {

    /*
     * header file
     */

    memset ((void *) file_label,
	    (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(file_label, STORM_HEADER_FILE_TYPE);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		s_handle->header_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      s_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing storm header file label.\n");
      perror(s_handle->header_file_path);
      return (R_FAILURE);
      
    }

    /*
     * data file
     */

    memset ((void *) file_label,
	    (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(file_label, STORM_DATA_FILE_TYPE);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		s_handle->data_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      s_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing storm data file label.\n");
      perror(s_handle->data_file_path);
      return (R_FAILURE);
      
    }

  } else {

    /*
     * read mode
     */


    if (ufread(file_label, (int) sizeof(char),
	       (int) R_FILE_LABEL_LEN,
	       s_handle->data_file) != R_FILE_LABEL_LEN) {
      fprintf(stderr, "ERROR - %s:%s:RfReadStormHeader\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr, "Reading data file label.\n");
      perror(s_handle->data_file_path);
      return (R_FAILURE);
    }
    
    if (s_handle->data_file_label == NULL) {
      
      s_handle->data_file_label = (char *) ucalloc
	((ui32) R_FILE_LABEL_LEN, (ui32) sizeof(char));
      memset ((void *)  s_handle->data_file_label,
	      (int) 0, (size_t)  R_FILE_LABEL_LEN);
      strcpy(s_handle->data_file_label, file_label);
      
    } /* if (s_handle->header_file_label != NULL) */
  
  } /* if (*mode == 'w') */

  /*
   * set header file path in index
   */
  
  if (s_handle->header_file_path != NULL)
    ufree ((char *) s_handle->header_file_path);

  s_handle->header_file_path = (char *) umalloc
    ((ui32) (strlen(header_file_path)+ 1));

  strcpy(s_handle->header_file_path, header_file_path);

  /*
   * set data file path in index
   */
  
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
 * RfReadStormHeader()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the storm_file_header_t structure from a storm
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

#define THIS_ROUTINE "RfReadStormheader"

int RfReadStormHeader(storm_file_handle_t *s_handle,
		      const char *calling_routine)
     
{
  
  char header_file_label[R_FILE_LABEL_LEN];
  char calling_sequence[MAX_SEQ];
  si32 nbytes_char;
  si32 n_scans;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate space for s_handle->file_header
   */
  
  if (!s_handle->header_allocated)
    if (RfAllocStormHeader(s_handle, calling_sequence))
      return (R_FAILURE);
  
  /*
   * rewind file
   */
  
  fseek(s_handle->header_file, 0L, SEEK_SET);
  
  /*
   * read in header file label
   */
  
  if (ufread(header_file_label, (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     s_handle->header_file) != R_FILE_LABEL_LEN) {
    fprintf(stderr, "ERROR - %s:%s:RfReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading header file label.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
  }
  
  /*
   * if the label passed in is non-null, check that the label matches
   * that expected. If the label is null, store the file label there.
   */
  
  if (s_handle->header_file_label != NULL) {
    
    if (strcmp(header_file_label, s_handle->header_file_label)) {
      fprintf(stderr, "ERROR - %s:%s:RfReadStormHeader\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "File does not contain correct type storm properties file.\n");
      fprintf(stderr, "File label is '%s'\n", header_file_label);
      fprintf(stderr, "File label should be '%s'.\n",
	      s_handle->header_file_label);
      
      return (R_FAILURE);
      
    }
    
  } else {
    
    s_handle->header_file_label = (char *) ucalloc
      ((ui32) R_FILE_LABEL_LEN, (ui32) sizeof(char));
    
    memset ((void *)  s_handle->header_file_label,
            (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(s_handle->header_file_label, header_file_label);
    
  } /* if (s_handle->header_file_label != NULL) */
  
  /*
   * read in header
   */
  
  if (ufread((char *) s_handle->header,
	     sizeof(storm_file_header_t),
	     1, s_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading storm file header structure.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the structure into host byte order - the file
   * is stored in network byte order
   */
  
  nbytes_char = s_handle->header->nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) s_handle->header,
	    (ui32) (sizeof(storm_file_header_t) - nbytes_char));
  
  /*
   * allocate space for scan offsets array
   */
  
  n_scans = s_handle->header->n_scans;
  
  if (RfAllocStormScanOffsets(s_handle, n_scans, calling_sequence))
    return (R_FAILURE);
  
  /*
   * read in scan offset
   */
  
  if (ufread((char *) s_handle->scan_offsets,
	     sizeof(si32),
	     (int) n_scans,
	     s_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading storm file scan offsets.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the offset array from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->scan_offsets,
	    (si32) (n_scans * sizeof(si32)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadStormProjRuns()
 *
 * reads in the storm projected area runs
 *
 * Space for the array is allocated.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadStormProjRuns"

int RfReadStormProjRuns(storm_file_handle_t *s_handle,
		     si32 storm_num, const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 n_proj_runs;
  
  /*
   * return early if nstorms is zero
   */
  
  if (s_handle->scan->nstorms == 0)
    return (R_SUCCESS);
  
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
   * allocate or realloc mem
   */
  
  n_proj_runs = s_handle->gprops[storm_num].n_proj_runs;
  
  if (RfAllocStormProps(s_handle, 0, 0, 0,
			n_proj_runs,
			calling_sequence))
    return (R_FAILURE);
  
  /*
   * move to proj_run data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  /*
   * read in proj_runs
   */
  
  if (ufread((char *) s_handle->proj_runs,
	     sizeof(storm_file_run_t),
	     (int) n_proj_runs,
	     s_handle->data_file) != n_proj_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormProjRuns\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading proj_runs - %ld runs.\n", (long) n_proj_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode proj_runs from network byte order into host byte order
   */
  
  BE_to_array_16((ui16 *) s_handle->proj_runs,
	    (ui32) (n_proj_runs * sizeof(storm_file_run_t)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadStormProps()
 *
 * reads in the storm property data for a given storm in a scan, given the
 * scan and index structures as inputs.
 *
 * Space for the arrays of structures is allocated as required.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadStormProps"

int RfReadStormProps(storm_file_handle_t *s_handle,
		     si32 storm_num, const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 n_layers, n_dbz_intervals;
  si32 n_runs, n_proj_runs;
  
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
   * allocate or realloc mem
   */
  
  n_layers = s_handle->gprops[storm_num].n_layers;
  n_dbz_intervals = s_handle->gprops[storm_num].n_dbz_intervals;
  n_runs = s_handle->gprops[storm_num].n_runs;
  n_proj_runs = s_handle->gprops[storm_num].n_proj_runs;
  
  if (RfAllocStormProps(s_handle, n_layers,
			n_dbz_intervals,
			n_runs, n_proj_runs,
			calling_sequence))
    return (R_FAILURE);
  
  /*
   * return early if nstorms is zero
   */
  
  if (s_handle->scan->nstorms == 0)
    return (R_SUCCESS);
  
  /*
   * move to layer data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].layer_props_offset, SEEK_SET);
  
  /*
   * read in layer props
   */
  
  if (ufread((char *) s_handle->layer,
	     sizeof(storm_file_layer_props_t),
	     (int) n_layers,
	     s_handle->data_file) != n_layers) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading layer props - %d layers.\n", n_layers);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode layer props from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->layer,
	    (ui32) (n_layers * sizeof(storm_file_layer_props_t)));
  
  
  /*
   * move to hist data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].dbz_hist_offset, SEEK_SET);
  
  /*
   * read in histogram data
   */
  
  if (ufread((char *) s_handle->hist,
	     sizeof(storm_file_dbz_hist_t),
	     (int) n_dbz_intervals,
	     s_handle->data_file) != n_dbz_intervals) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading dbz histogram - %d intervals.\n",
	    n_dbz_intervals);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode histogram data from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->hist,
	    (ui32) (n_dbz_intervals * sizeof(storm_file_dbz_hist_t)));
  
  /*
   * move to run data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].runs_offset, SEEK_SET);
  
  /*
   * read in runs
   */
  
  if (ufread((char *) s_handle->runs,
	     sizeof(storm_file_run_t),
	     (int) n_runs,
	     s_handle->data_file) != n_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading runs - %ld runs.\n", (long) n_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode runs from network byte order into host byte order
   */
  
  BE_to_array_16((ui16 *) s_handle->runs,
	    (ui32) (n_runs * sizeof(storm_file_run_t)));
  
  /*
   * move to proj_run data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  /*
   * read in proj_runs
   */
  
  if (ufread((char *) s_handle->proj_runs,
	     sizeof(storm_file_run_t),
	     (int) n_proj_runs,
	     s_handle->data_file) != n_proj_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading proj_runs - %ld runs.\n", (long) n_proj_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode proj_runs from network byte order into host byte order
   */
  
  BE_to_array_16((ui16 *) s_handle->proj_runs,
	    (ui32) (n_proj_runs * sizeof(storm_file_run_t)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadStormScan()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the scan info for a particular scan in a storm properties
 * file.
 *
 * Space for the scan structure and the storm global props structure is
 * allocated
 * if acsn_allocated is FALSE. A check is kept on the max number of storms for
 * which space is allocated. If this is exceeded during a call in which space
 * is not to be allocated, the allocation is increased.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadStormScan"

int RfReadStormScan(storm_file_handle_t *s_handle, si32 scan_num,
		    const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nbytes_char;
  si32 nstorms;
  storm_file_scan_header_t scan;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * move to scan position in file
   */
  
  if (s_handle->scan_offsets) {
    fseek(s_handle->data_file,
	  s_handle->scan_offsets[scan_num], SEEK_SET);
  } else {
    return (R_FAILURE);
  }
  
  /*
   * read in scan struct
   */
  
  if (ufread((char *) &scan,
	     sizeof(storm_file_scan_header_t),
	     1, s_handle->data_file) != 1) {

    fprintf(stderr, "ERROR - %s:%s:RfReadStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading scan number %d\n", scan_num);
    perror(s_handle->data_file_path);

    return(R_FAILURE);
    
  }
  
  /*
   * decode the scan struct from network byte order into host byte order
   */
  
  nbytes_char = scan.nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) &scan,
	    (ui32) (sizeof(storm_file_scan_header_t) - nbytes_char));
  
  nstorms = scan.nstorms;
  
  /*
   * allocate or reallocate
   */
  
  if (RfAllocStormScan(s_handle, nstorms, calling_sequence))
    return (R_FAILURE);
  
  /*
   * copy scan header into storm file index
   */
  
  memcpy ((void *) s_handle->scan,
          (void *) &scan,
          (size_t) sizeof(storm_file_scan_header_t));
  
  /*
   * return early if nstorms is zero
   */
  
  if (nstorms == 0)
    return (R_SUCCESS);
  
  /*
   * move to gprops position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->scan->gprops_offset, SEEK_SET);
  
  /*
   * read in global props
   */
  
  if (ufread((char *) s_handle->gprops,
	     sizeof(storm_file_global_props_t),
	     (int) nstorms, s_handle->data_file) != nstorms) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading global props for scan number %d\n", scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode global props from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->gprops,
	    (ui32) (nstorms * sizeof(storm_file_global_props_t)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfSeekEndStormData()
 *
 * part of the rfutil library - radar file access
 *
 * seeks to the end of the storm data in data file
 *
 **************************************************************************/

int RfSeekEndStormData(storm_file_handle_t *s_handle,
		       const char *calling_routine)
     
{

  if (fseek(s_handle->data_file,
	    (si32) R_FILE_LABEL_LEN,
	    SEEK_END)) {
    
    fprintf(stderr, "ERROR - %s:%s:RfSeekEndStormData\n",
	    s_handle->prog_name, calling_routine);
    
    fprintf(stderr, "Failed on seek.\n");
    perror(s_handle->data_file_path);

    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * RfSeekStartStormData()
 *
 * part of the rfutil library - radar file access
 *
 * seeks to the start of the storm data in data file
 *
 **************************************************************************/

int RfSeekStartStormData(storm_file_handle_t *s_handle,
			 const char *calling_routine)
     
{

  if (fseek(s_handle->data_file,
	    (si32) R_FILE_LABEL_LEN,
	    SEEK_SET) != 0) {
    
    fprintf(stderr, "ERROR - %s:%s:RfSeekStartStormData\n",
	    s_handle->prog_name, calling_routine);
    
    fprintf(stderr, "Failed on seek.\n");
    perror(s_handle->data_file_path);

    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * RfWriteStormHeader()
 *
 * part of the rfutil library - radar file access
 *
 * writes the storm_file_header_t structure to a storm
 * properties file.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteStormHeader"

int RfWriteStormHeader(storm_file_handle_t *s_handle,
		       const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[R_FILE_LABEL_LEN];
  char *cptr;
  storm_file_header_t header;
  si32 *scan_offsets;
  si32 n_scans;
  struct stat data_stat;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * get data file size
   */

  fflush(s_handle->data_file);
  stat (s_handle->data_file_path, &data_stat);
  s_handle->header->data_file_size = data_stat.st_size;
  
  /*
   * copy file label
   */
  
  memset ((void *) file_label,
          (int) 0, (size_t)  R_FILE_LABEL_LEN);
  strcpy(file_label, STORM_HEADER_FILE_TYPE);

  s_handle->header->major_rev = 5;
  s_handle->header->minor_rev = 1;
  
  /*
   * set file time to gmt
   */
  
  s_handle->header->file_time = time(NULL);

  /*
   * copy in the file names, checking whether the path has a
   * delimiter or not, and only copying after the delimiter
   */

  if ((cptr = strrchr(s_handle->header_file_path, '/')) != NULL)
    strncpy(s_handle->header->header_file_name, (cptr + 1), R_LABEL_LEN);
  else
    strncpy(s_handle->header->header_file_name,
	    s_handle->header_file_path, R_LABEL_LEN);
      
  
  if ((cptr = strrchr(s_handle->data_file_path, '/')) != NULL)
    strncpy(s_handle->header->data_file_name, (cptr + 1), R_LABEL_LEN);
  else
    strncpy(s_handle->header->data_file_name,
	    s_handle->data_file_path, R_LABEL_LEN);
      
  
  /*
   * make local copies of the global file header and scan offsets
   */
  
  memcpy ((void *) &header,
          (void *) s_handle->header,
          (size_t) sizeof(storm_file_header_t));
  
  n_scans = s_handle->header->n_scans;
  
  scan_offsets = (si32 *) umalloc
    ((ui32) (n_scans * sizeof(si32)));
  
  memcpy ((void *) scan_offsets,
          (void *) s_handle->scan_offsets,
          (size_t) (n_scans * sizeof(si32)));
  
  /*
   * encode the header and scan offset array into network byte order
   */
  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  header.nbytes_char = STORM_FILE_HEADER_NBYTES_CHAR;
  BE_from_array_32((ui32 *) &header,
	      (ui32) (sizeof(storm_file_header_t) - header.nbytes_char));
  BE_from_array_32((ui32 *) scan_offsets,
	      (si32) (n_scans * sizeof(si32)));
  
  /*
   * write label to file
   */
  
  fseek(s_handle->header_file, (si32) 0, SEEK_SET);
  ustr_clear_to_end(file_label, R_FILE_LABEL_LEN);

  if (ufwrite(file_label, (int) sizeof(char),
	      (int) R_FILE_LABEL_LEN,
	      s_handle->header_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm header file label.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write header to file
   */
  
  if (ufwrite((char *) &header,
	      sizeof(storm_file_header_t),
	      1, s_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm file header structure.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write scan offsets to file
   */
  
  if (ufwrite((char *) scan_offsets,
	      sizeof(si32),
	      (int) n_scans,
	      s_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm file scan offsets.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * flush the file buffer
   */
  
  if (RfFlushStormFiles(s_handle, calling_sequence)) {
    return (R_FAILURE);
  }

  /*
   * free resources
   */

  ufree ((char *) scan_offsets);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfWriteStormProps()
 *
 * part of the rfutil library - radar file access
 *
 * writes the storm layer property and histogram data for a storm, at
 * the end of the file.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteStormProps(storm_file_handle_t *s_handle, si32 storm_num,
		      const char *calling_routine)
     
{
  
  si32 offset;
  si32 n_layers, n_dbz_intervals, n_runs, n_proj_runs;
  storm_file_layer_props_t *layer;
  storm_file_dbz_hist_t *hist;
  storm_file_run_t *runs;
  storm_file_run_t *proj_runs;
  
  n_layers = s_handle->gprops[storm_num].n_layers;
  n_dbz_intervals = s_handle->gprops[storm_num].n_dbz_intervals;
  n_runs = s_handle->gprops[storm_num].n_runs;
  n_proj_runs = s_handle->gprops[storm_num].n_proj_runs;
  
  /*
   * set layer props offset
   */
  
  fseek(s_handle->data_file, 0, SEEK_END);
  offset = ftell(s_handle->data_file);
  s_handle->gprops[storm_num].layer_props_offset = offset;
  
  /*
   * if this is the first storm, store the first_offset value
   * in the scan header
   */
  
  if (storm_num == 0)
    s_handle->scan->first_offset = offset;
  
  /*
   * copy layer props to local variable
   */
  
  layer = (storm_file_layer_props_t *)
    umalloc ((ui32) (n_layers * sizeof(storm_file_layer_props_t)));
  
  memcpy ((void *)  layer,
          (void *)  s_handle->layer,
          (size_t)  (n_layers * sizeof(storm_file_layer_props_t)));
  
  /*
   * code layer props into network byte order from host byte order
   */
  
  BE_from_array_32((ui32 *) layer,
	      (ui32) (n_layers * sizeof(storm_file_layer_props_t)));
  
  /*
   * write layer props
   */
  
  if (ufwrite((char *) layer,
	      sizeof(storm_file_layer_props_t),
	      (int) n_layers, s_handle->data_file) != n_layers) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing layer props - %d layers.\n", n_layers);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * set dbz hist offset
   */
  
  s_handle->gprops[storm_num].dbz_hist_offset = ftell(s_handle->data_file);
  
  /*
   * copy histogram data to local variable
   */
  
  hist = (storm_file_dbz_hist_t *)
    umalloc((ui32) n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  memcpy ((void *)  hist,
          (void *)  s_handle->hist,
          (size_t)  (n_dbz_intervals * sizeof(storm_file_dbz_hist_t)));
  
  /*
   * encode histogram data to network byte order from host byte order
   */
  
  BE_from_array_32((ui32 *) hist,
	      (ui32) (n_dbz_intervals * sizeof(storm_file_dbz_hist_t)));
  
  /*
   * write in histogram data
   */
  
  if (ufwrite((char *) hist,
	      sizeof(storm_file_dbz_hist_t),
	      (int) n_dbz_intervals,
	      s_handle->data_file) != n_dbz_intervals) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing dbz histogram - %d intervals.\n",
	    n_dbz_intervals);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * set runs offset
   */
  
  s_handle->gprops[storm_num].runs_offset = ftell(s_handle->data_file);
  
  /*
   * copy runs to local variable
   */
  
  runs = (storm_file_run_t *)
    umalloc ((ui32) (n_runs * sizeof(storm_file_run_t)));
  
  memcpy ((void *)  runs,
          (void *)  s_handle->runs,
          (size_t)  (n_runs * sizeof(storm_file_run_t)));
  
  /*
   * code run props into network byte order from host byte order
   */
  
  BE_from_array_16((ui16 *) runs,
	      (ui32) (n_runs * sizeof(storm_file_run_t)));
  
  /*
   * write runs
   */
  
  if (ufwrite((char *) runs,
	      sizeof(storm_file_run_t),
	      (int) n_runs, s_handle->data_file) != n_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing runs - %ld runs.\n", (long) n_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * set proj_runs offset
   */
  
  s_handle->gprops[storm_num].proj_runs_offset = ftell(s_handle->data_file);
  
  /*
   * copy proj_runs to local variable
   */
  
  proj_runs = (storm_file_run_t *)
    umalloc ((ui32) (n_proj_runs * sizeof(storm_file_run_t)));
  
  memcpy ((void *)  proj_runs,
          (void *)  s_handle->proj_runs,
          (size_t)  (n_proj_runs * sizeof(storm_file_run_t)));
  
  /*
   * code run props into network byte order from host byte order
   */
  
  BE_from_array_16((ui16 *) proj_runs,
	      (ui32) (n_proj_runs * sizeof(storm_file_run_t)));
  
  /*
   * write proj_runs
   */
  
  if (ufwrite((char *) proj_runs,
	      sizeof(storm_file_run_t),
	      (int) n_proj_runs, s_handle->data_file) != n_proj_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing proj_runs - %ld runs.\n", (long) n_proj_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }

  /*
   * free up
   */

  ufree_non_null((void **) &layer);
  ufree_non_null((void **) &hist);
  ufree_non_null((void **) &runs);
  ufree_non_null((void **) &proj_runs);
  
  return (R_SUCCESS);
  
}


/*************************************************************************
 *
 * RfWriteStormScan()
 *
 * part of the rfutil library - radar file access
 *
 * writes scan header and global properties for a particular scan in a storm
 * properties file. Performs the writes from the end of the file.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteStormScan"

int RfWriteStormScan(storm_file_handle_t *s_handle, si32 scan_num,
		     const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nstorms;
  si32 offset;
  storm_file_scan_header_t scan;
  storm_file_global_props_t *gprops;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * Move to the end of the file before beginning the write.
   */

  fseek(s_handle->data_file, 0, SEEK_END);

  /*
   * if nstorms is greater than zero, write global props to file
   */
  
  nstorms = s_handle->scan->nstorms;
  
  if (nstorms > 0) {
    
    /*
     * get gprops position in file
     */
    
    s_handle->scan->gprops_offset = ftell(s_handle->data_file);
    
    /*
     * allocate space for global props copy
     */
    
    gprops = (storm_file_global_props_t *)
      umalloc((ui32) (nstorms * sizeof(storm_file_global_props_t)));
    
    /*
     * make local copy of gprops and encode into network byte order
     */
    
    memcpy ((void *)  gprops,
            (void *)  s_handle->gprops,
            (size_t)  nstorms * sizeof(storm_file_global_props_t));
    
    BE_from_array_32((ui32 *) gprops,
	      (ui32) (nstorms * sizeof(storm_file_global_props_t)));
    
    /*
     * write in global props
     */
    
    if (ufwrite((char *) gprops,
		sizeof(storm_file_global_props_t),
		(int) nstorms, s_handle->data_file) != nstorms) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteStormScan\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr, "Writing global props for scan number %d\n", scan_num);
      perror(s_handle->data_file_path);
      return(R_FAILURE);
      
    }
    
    /*
     * free up global props copy
     */
    
    ufree(gprops);
    
  } /* if (nstorms > 0) */
  
  /*
   * get scan position in file
   */
  
  if (RfAllocStormScanOffsets(s_handle, (si32) (scan_num + 1),
			      calling_sequence))
    return (R_FAILURE);

  offset = ftell(s_handle->data_file);
  s_handle->scan_offsets[scan_num] = offset;
  
  /*
   * set last scan offset
   */
  
  s_handle->scan->last_offset = offset + sizeof(storm_file_scan_header_t) - 1;
  
  /*
   * copy scan header to local variable, and encode. Note that the 
   * character data at the end of the struct is not encoded
   */
  
  memcpy ((void *)  &scan,
          (void *)  s_handle->scan,
          (size_t)  sizeof(storm_file_scan_header_t));
  
  scan.grid.nbytes_char =  TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  scan.nbytes_char = scan.grid.nbytes_char;
  
  ustr_clear_to_end(scan.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32((ui32 *) &scan,
	      (ui32) (sizeof(storm_file_scan_header_t) - scan.nbytes_char));
  
  /*
   * write in scan struct
   */
  
  if (ufwrite((char *) &scan,
	      sizeof(storm_file_scan_header_t), 1,
	      s_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing scan number %d\n", scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/********************************************************
 * routine to convert ellipse parameters from deg to km,
 * for those which were computed from latlon grids.
 */

static void _convert_ellipse_2km(titan_grid_t *tgrid,
				 double centroid_x,
				 double centroid_y,
				 fl32 *orientation,
				 fl32 *minor_radius,
				 fl32 *major_radius)
     
{

  /*
   * only convert for latlon projection
   */
  
  if (tgrid->proj_type == TITAN_PROJ_LATLON) {
    
    double centroid_lon, centroid_lat;
    double major_orient_rad, major_lon, major_lat;
    double minor_orient_rad, minor_lon, minor_lat;
    double dist, theta;
    double orientation_km, major_radius_km, minor_radius_km;
    double sin_major, cos_major;
    double sin_minor, cos_minor;

    centroid_lon = centroid_x;
    centroid_lat = centroid_y;
    
    major_orient_rad = *orientation * DEG_TO_RAD;
    ta_sincos(major_orient_rad, &sin_major, &cos_major);
    major_lon = centroid_lon + *major_radius * sin_major;
    major_lat = centroid_lat + *major_radius * cos_major;
    
    minor_orient_rad = (*orientation + 270.0) * DEG_TO_RAD;
    ta_sincos(minor_orient_rad, &sin_minor, &cos_minor);
    minor_lon = centroid_lon + *minor_radius * sin_minor;
    minor_lat = centroid_lat + *minor_radius * cos_minor;

    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     major_lat, major_lon,
		     &dist, &theta);

    orientation_km = theta;
    major_radius_km = dist;
    
    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     minor_lat, minor_lon,
		     &dist, &theta);
    
    minor_radius_km = dist;
    
    *orientation = orientation_km;
    *major_radius = major_radius_km;
    *minor_radius = minor_radius_km;

  }

}
		     
/*************************************************************************
 *
 * RfStormGprops2Km()
 *
 * Convert the ellipse data (orientation, major_radius and minor_radius)
 * for a a gprops struct to local (km) values.
 * This applies to structs which were derived from lat-lon grids, for
 * which some of the fields are in deg instead of km.
 * It is a no-op for other projections.
 *
 * See Note 3 in storms.h
 *
 **************************************************************************/

void RfStormGpropsEllipses2Km(storm_file_scan_header_t *scan,
			      storm_file_global_props_t *gprops)
     
{
  
  titan_grid_t  *tgrid = &scan->grid;

  /*
   * convert the ellipses as appropriate
   */

  _convert_ellipse_2km(tgrid,
		       gprops->precip_area_centroid_x,
		       gprops->precip_area_centroid_y,
		       &gprops->precip_area_orientation,
		       &gprops->precip_area_minor_radius,
		       &gprops->precip_area_major_radius);
  
  _convert_ellipse_2km(tgrid,
		       gprops->proj_area_centroid_x,
		       gprops->proj_area_centroid_y,
		       &gprops->proj_area_orientation,
		       &gprops->proj_area_minor_radius,
		       &gprops->proj_area_major_radius);

}

/*************************************************************************
 *
 * RfStormGpropsXY2LatLon()
 *
 * Convert the (x,y) km locations in a gprops struct to lat-lon.
 * This applies to structs which were computed for non-latlon 
 * grids. It is a no-op for lat-lon grids.
 *
 * See Note 3 in storms.h
 *
 **************************************************************************/

void RfStormGpropsXY2LatLon(storm_file_scan_header_t *scan,
			    storm_file_global_props_t *gprops)
     
{
  
  titan_grid_t  *tgrid = &scan->grid;

  switch (tgrid->proj_type) {
    
  case TITAN_PROJ_LATLON:
    break;
    
  case TITAN_PROJ_FLAT:
    {
      double lat, lon;
      PJGLatLonPlusDxDy(tgrid->proj_origin_lat,
			tgrid->proj_origin_lon,
			gprops->vol_centroid_x,
			gprops->vol_centroid_y,
			&lat, &lon);
      gprops->vol_centroid_y = lat;
      gprops->vol_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid->proj_origin_lat,
			tgrid->proj_origin_lon,
			gprops->refl_centroid_x,
			gprops->refl_centroid_y,
			&lat, &lon);
      gprops->refl_centroid_y = lat;
      gprops->refl_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid->proj_origin_lat,
			tgrid->proj_origin_lon,
			gprops->precip_area_centroid_x,
			gprops->precip_area_centroid_y,
			&lat, &lon);
      gprops->precip_area_centroid_y = lat;
      gprops->precip_area_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid->proj_origin_lat,
			tgrid->proj_origin_lon,
			gprops->proj_area_centroid_x,
			gprops->proj_area_centroid_y,
			&lat, &lon);
      gprops->proj_area_centroid_y = lat;
      gprops->proj_area_centroid_x = lon;
      break;
    }
    
  case TITAN_PROJ_LAMBERT_CONF:
    {
      double lat, lon;
      PJGstruct *ps = PJGs_lc2_init(tgrid->proj_origin_lat,
				    tgrid->proj_origin_lon,
				    tgrid->proj_params.lc2.lat1,
				    tgrid->proj_params.lc2.lat2);
      if (ps != NULL) {
	PJGs_lc2_xy2latlon(ps,
			   gprops->vol_centroid_x,
			   gprops->vol_centroid_y,
			   &lat, &lon);
	gprops->vol_centroid_y = lat;
	gprops->vol_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops->refl_centroid_x,
			   gprops->refl_centroid_y,
			   &lat, &lon);
	gprops->refl_centroid_y = lat;
	gprops->refl_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops->precip_area_centroid_x,
			   gprops->precip_area_centroid_y,
			   &lat, &lon);
	gprops->precip_area_centroid_y = lat;
	gprops->precip_area_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops->proj_area_centroid_x,
			   gprops->proj_area_centroid_y,
			   &lat, &lon);
	gprops->proj_area_centroid_y = lat;
	gprops->proj_area_centroid_x = lon;
	free(ps);
      }
      break;
    }
  
  default:
    break;
    
  } /* switch */
  
}




