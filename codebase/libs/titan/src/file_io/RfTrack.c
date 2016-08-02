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
 * RfTrack.c
 *
 * Track file access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * October 1991
 *
 **************************************************************************/

#include <titan/track.h>
#include <dataport/bigend.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_SEQ 256
#define N_ALLOC 5

/*************************************************************************
 *
 * RfAllocComplexTrackParams()
 *
 * allocates parameters struct for a complex track
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocComplexTrackParams(track_file_handle_t *t_handle,
			      const char *calling_routine)
     
{

  if (!t_handle->complex_params_allocated) {
    
    t_handle->complex_params = (complex_track_params_t *)
      ucalloc((ui32) 1, (ui32) sizeof(complex_track_params_t));
    
    t_handle->complex_params_allocated = TRUE;
    
  } /* if (!t_handle->complex_params_allocated) */
  
  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocSimplesPerComplex()
 *
 * allocates space for the array of pointers to simples_per_complex
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocSimplesPerComplex(track_file_handle_t *t_handle,
			     si32 n_simple_needed,
			     const char *calling_routine)
     
{

  si32 n_realloc;
  si32 n_start, n_new;

  if (t_handle->n_simples_per_complex_allocated < n_simple_needed) {
    
    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
    
    n_start = t_handle->n_simples_per_complex_allocated;

    if (t_handle->n_simples_per_complex_allocated == 0) {
      
      t_handle->n_simples_per_complex_allocated = n_simple_needed + N_ALLOC;
      
      t_handle->simples_per_complex = (si32 **) umalloc
	((ui32) (t_handle->n_simples_per_complex_allocated * sizeof(si32 *)));
      
    } else {

      n_realloc = n_simple_needed + N_ALLOC;

      t_handle->n_simples_per_complex_allocated = n_realloc;

      t_handle->simples_per_complex = (si32 **) urealloc
	((char *) t_handle->simples_per_complex,
	 (ui32) (n_realloc * sizeof(si32 *)));
      
    } /* if (t_handle->n_simples_per_complex_allocated == 0) */

    /*
     * initialize new elements to zero
     */
  
    n_new = t_handle->n_simples_per_complex_allocated - n_start;

    memset ((void *) (t_handle->simples_per_complex + n_start),
	    (int) 0, (int) (n_new * sizeof(si32 *)));
  
  } /* if (t_handle->n_simples_per_complex_allocated < n_simple_needed) */

  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfAllocSimpleTrackParams()
 *
 * allocates parameters struct for a complex track
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocSimpleTrackParams(track_file_handle_t *t_handle,
			     const char *calling_routine)
     
{

  if (!t_handle->simple_params_allocated) {
    
    t_handle->simple_params = (simple_track_params_t *)
      ucalloc((ui32) 1, (ui32) sizeof(simple_track_params_t));
    
    t_handle->simple_params_allocated = TRUE;
    
  } /* if (!t_handle->simple_params_allocated) */
  
  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocTrackComplexArrays()
 *
 * allocates space for the complex track arrays.
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocTrackComplexArrays(track_file_handle_t *t_handle,
			      si32 n_complex_needed,
			      const char *calling_routine)
     
{

  si32 n_realloc;
  si32 n_start;
  si32 n_new;
  
  n_start = t_handle->n_complex_allocated;

  if (t_handle->n_complex_allocated < n_complex_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
    
    if (t_handle->n_complex_allocated == 0) {
      
      t_handle->n_complex_allocated = n_complex_needed + N_ALLOC;
      
      t_handle->complex_track_nums = (si32 *) umalloc
	((ui32) (t_handle->n_complex_allocated * sizeof(si32)));
      
    } else {

      n_realloc = n_complex_needed + N_ALLOC;
      t_handle->n_complex_allocated = n_realloc;

      t_handle->complex_track_nums = (si32 *) urealloc
	((char *) t_handle->complex_track_nums,
	 (ui32) (n_realloc * sizeof(si32)));

    } /* if (t_handle->n_complex_allocated == 0) */

    /*
     * initialize new elements to zero
     */
  
    n_new = t_handle->n_complex_allocated - n_start;

    memset ((void *) (t_handle->complex_track_nums + n_start),
	    (int) 0, (int) (n_new * sizeof(si32)));
  
  } /* if (t_handle->n_complex_allocated < n_complex_needed) */

  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfAllocTrackEntry()
 *
 * allocates mem for a track entry struct
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocTrackEntry(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{
  
  if (!t_handle->entry_allocated) {
    
    t_handle->entry = (track_file_entry_t *)
      ucalloc((ui32) 1, (ui32) sizeof(track_file_entry_t));
    
    t_handle->entry_allocated = TRUE;
    
  } /* if (!t_handle->entry_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocTrackHeader()
 *
 * allocates space for the track_file_header_t structure and the
 * track offset arrays.
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocTrackHeader(track_file_handle_t *t_handle,
		       const char *calling_routine)
     
{

  if (!t_handle->header_allocated) {

    t_handle->header = (track_file_header_t *)
      ucalloc((ui32) 1, (ui32) sizeof(track_file_header_t));

    t_handle->header_allocated = TRUE;

  } /* if (!t_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocTrackScanEntries()
 *
 * allocates mem for scan entries array
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocTrackScanEntries(track_file_handle_t *t_handle,
			    const char *calling_routine)
     
{
  
  if (!t_handle->scan_entries_allocated) {
    
    t_handle->scan_entries = (track_file_entry_t *)
      ucalloc((ui32) t_handle->n_scan_entries,
	      (ui32) sizeof(track_file_entry_t));
    
    t_handle->scan_entries_allocated = TRUE;
    t_handle->n_scan_entries_allocated = t_handle->n_scan_entries;
    
  } else {

    if (t_handle->n_scan_entries > t_handle->n_scan_entries_allocated) {

      t_handle->scan_entries = (track_file_entry_t *) urealloc
	((char *) t_handle->scan_entries,
	 (ui32) (t_handle->n_scan_entries * sizeof(track_file_entry_t)));
    
      t_handle->n_scan_entries_allocated = t_handle->n_scan_entries;

    } /* if (t_handle->n_scan_entries > t_handle->n_scan_entries_allocated) */

  } /* if (!tdindex->arrays_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfAllocTrackScanIndex()
 *
 * allocates space for the simple track arrays.
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocTrackScanIndex(track_file_handle_t *t_handle,
			  si32 n_scans_needed,
			  const char *calling_routine)
     
{

  si32 n_realloc;
  si32 n_start, n_new;
  
  if (t_handle->n_scans_allocated < n_scans_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
      
    n_start = t_handle->n_scans_allocated;

    if (t_handle->n_scans_allocated == 0) {
      
      t_handle->n_scans_allocated = n_scans_needed + N_ALLOC;
      
      t_handle->scan_index = (track_file_scan_index_t *) umalloc
	((ui32) (t_handle->n_scans_allocated *
		 sizeof(track_file_scan_index_t)));
      
    } else {
      
      n_realloc = n_scans_needed + N_ALLOC;
      t_handle->n_scans_allocated = n_realloc;

      t_handle->scan_index = (track_file_scan_index_t *) urealloc
	((char *) t_handle->scan_index,
	 (ui32) (n_realloc * sizeof(track_file_scan_index_t)));
      
    } /* if (t_handle->n_scans_allocated == 0) */

    /*
     * initialize new elements to zero
     */
  
    n_new = t_handle->n_scans_allocated - n_start;

    memset ((void *) (t_handle->scan_index + n_start),
	    (int) 0,
	    (int) (n_new * sizeof(track_file_scan_index_t)));
  
  } /* if (t_handle->n_scans_allocated < n_scans_needed) */

  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfAllocTrackSimpleArrays()
 *
 * allocates space for the simple track arrays.
 *
 *************************************************************************/

/*ARGSUSED*/

int RfAllocTrackSimpleArrays(track_file_handle_t *t_handle,
			     si32 n_simple_needed,
			     const char *calling_routine)
     
{

  si32 n_realloc;
  si32 n_start, n_new;

  if (t_handle->n_simple_allocated < n_simple_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
    
    n_start = t_handle->n_simple_allocated;

    if (t_handle->n_simple_allocated == 0) {
      
      t_handle->n_simple_allocated = n_simple_needed + N_ALLOC;
      
      t_handle->simple_track_offsets = (si32 *) umalloc
	((ui32) (t_handle->n_simple_allocated * sizeof(si32)));
      
      t_handle->nsimples_per_complex = (si32 *) umalloc
	((ui32) (t_handle->n_simple_allocated * sizeof(si32)));
      
      t_handle->simples_per_complex_offsets = (si32 *) umalloc
	((ui32) (t_handle->n_simple_allocated * sizeof(si32)));
      
      t_handle->complex_track_offsets = (si32 *) umalloc
	((ui32) (t_handle->n_simple_allocated * sizeof(si32)));
      
    } else {

      n_realloc = n_simple_needed + N_ALLOC;
      t_handle->n_simple_allocated = n_realloc;

      t_handle->simple_track_offsets = (si32 *) urealloc
	((char *) t_handle->simple_track_offsets,
	 (ui32) (n_realloc * sizeof(si32)));
      
      t_handle->nsimples_per_complex = (si32 *) urealloc
	((char *) t_handle->nsimples_per_complex,
	 (ui32) (n_realloc * sizeof(si32)));
      
      t_handle->simples_per_complex_offsets = (si32 *) urealloc
	((char *) t_handle->simples_per_complex_offsets,
	 (ui32) (n_realloc * sizeof(si32)));
      
      t_handle->complex_track_offsets = (si32 *) urealloc
	((char *) t_handle->complex_track_offsets,
	 (ui32) (n_realloc * sizeof(si32)));
      
    } /* if (t_handle->n_simple_allocated == 0) */

    /*
     * initialize new elements to zero
     */
  
    n_new = t_handle->n_simple_allocated - n_start;

    memset ((void *) (t_handle->simple_track_offsets + n_start),
	    (int) 0, (int) (n_new * sizeof(si32)));
  
    memset ((void *) (t_handle->nsimples_per_complex + n_start),
	    (int) 0, (int) (n_new * sizeof(si32)));
  
    memset ((void *) (t_handle->simples_per_complex_offsets + n_start),
	    (int) 0, (int) (n_new * sizeof(si32)));
  
    memset ((void *) (t_handle->complex_track_offsets + n_start),
	    (int) 0, (int) (n_new * sizeof(si32)));
  
  } /* if (t_handle->n_simple_allocated < n_simple_needed) */

  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfAllocTrackUtime()
 *
 * allocates array of track_utime_t structs
 *
 * Returns R_SUCCESS or R_FAILURE
 *
 **************************************************************************/

/*ARGSUSED*/

void RfAllocTrackUtime(track_file_handle_t *t_handle,
		       const char *calling_routine)
     
{

  if (!t_handle->track_utime_allocated) {

    /*
     * allocate array
     */
    
    t_handle->n_track_utime_allocated =
      t_handle->header->max_simple_track_num + 1;
    
    t_handle->track_utime = (track_utime_t *) ucalloc
      ((ui32) t_handle->n_track_utime_allocated,
       (ui32) sizeof(track_utime_t));
    
    t_handle->track_utime_allocated = TRUE;

  } else {
    
    if (t_handle->n_track_utime_allocated <
	t_handle->header->max_simple_track_num + 1) {
      
      /* 
       * reallocate track_utime array
       */
      
      t_handle->n_track_utime_allocated =
	t_handle->header->max_simple_track_num + 1;
      
      t_handle->track_utime = (track_utime_t *) urealloc
	((char *) t_handle->track_utime,
	 (ui32) (t_handle->n_track_utime_allocated *
		 sizeof(track_utime_t)));
      
      memset ((void *)  t_handle->track_utime,
              (int) 0, (size_t)  (t_handle->n_track_utime_allocated *
				  sizeof(track_utime_t)));
      
    } /* if (t_handle->n_track_utime_allocated ... */

  } /* if (!t_handle->track_utime_allocated) */

}

/*****************************************************************************
 * RfCheckTrackMaxValues()
 *
 * checks that max value in the header does not exceed tha value
 * currently in use in the 'track.h' header file
 *
 *****************************************************************************/

int RfCheckTrackMaxValues(track_file_handle_t *t_handle,
			  const char *calling_routine,
			  si32 file_header_max,
			  si32 declared_max,
			  const char *description_small,
			  const char *description_caps)
     
{
  
  if (file_header_max > declared_max) {
    
    fprintf(stderr, "ERROR - %s:%s\n",
	    t_handle->prog_name, calling_routine);
    
    fprintf(stderr,
	    "Track file header '%s' exceeds current value.\n",
	    description_small);
    
    fprintf(stderr,
	    "'%s' in file is %ld\n",
	    description_small, (long) file_header_max);
    
    fprintf(stderr,
	    "'%s' in use now is %ld\n",
	    description_caps, (long) declared_max);
    
    fprintf(stderr,
	    "Increase '%s' in 'track.h' to %ld, and remake all.\n",
	    description_caps,
	    (long) file_header_max);
    
    return (R_FAILURE);
    
  } else {
    
    return (R_SUCCESS);
    
  }
  
}

/*************************************************************************
 *
 * RfCloseTrackFiles()
 *
 * Closes the track header and data files
 *
 **************************************************************************/

int RfCloseTrackFiles(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{
 
  /*
   * close the header file
   */

  if (t_handle->header_file != NULL) {
    if (fclose(t_handle->header_file)) {
      fprintf(stderr, "WARNING - %s:RfCloseTrackFiles\n", calling_routine);
      perror(t_handle->header_file_path);
    }
    t_handle->header_file = (FILE *) NULL;
  }

  /*
   * close the data file
   */
  
  if (t_handle->data_file != NULL) {
    if (fclose(t_handle->data_file)) {
      fprintf(stderr, "WARNING - %s:RfCloseTrackFiles\n", calling_routine);
      perror(t_handle->data_file_path);
    }
    t_handle->data_file = (FILE *) NULL;
  }

  /*
   * free up resources
   */

  if (t_handle->header_file_path != NULL) {
    ufree ((char *) t_handle->header_file_path);
    t_handle->header_file_path = (char *) NULL;
  }

  if (t_handle->data_file_path != NULL) {
    ufree ((char *) t_handle->data_file_path);
    t_handle->data_file_path = (char *) NULL;
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFlushTrackFiles()
 *
 * Flushes the track header and data files
 *
 **************************************************************************/

int RfFlushTrackFiles(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{
 
  /*
   * flush the header file
   */
  
  if (fflush(t_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:RfFlushTrackFiles\n", calling_routine);
    fprintf(stderr, "Flushing track header file.\n");
    perror(t_handle->header_file_path);
  }

  /*
   * flush the data file
   */
  
  if (fflush(t_handle->data_file)) {
    fprintf(stderr, "WARNING - %s:RfFlushTrackFiles\n", calling_routine);
    fprintf(stderr, "Flushing track data file.\n");
    perror(t_handle->data_file_path);
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeTrackArrays()
 *
 * frees arrays for track file index
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeTrackArrays(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{

  int i;
  
  if (t_handle->n_complex_allocated) {
    ufree(t_handle->complex_track_nums);
    t_handle->n_complex_allocated = 0;
  }

  if (t_handle->n_simple_allocated) {
    for (i = 0; i < t_handle->n_simple_allocated; i++) {
      if(t_handle->simples_per_complex[i] != NULL) {
	ufree(t_handle->simples_per_complex[i]);
      }
    }
    ufree(t_handle->simple_track_offsets);
    ufree(t_handle->nsimples_per_complex);
    ufree(t_handle->simples_per_complex_offsets);
    ufree(t_handle->simples_per_complex);
    ufree(t_handle->complex_track_offsets);
    t_handle->n_simple_allocated = 0;
  }

  if (t_handle->n_scans_allocated) {
    ufree(t_handle->scan_index);
    t_handle->n_scans_allocated = 0;
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeTrackEntry()
 *
 * frees entry for a track
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeTrackEntry(track_file_handle_t *t_handle,
		     const char *calling_routine)
     
{

  if (t_handle->entry_allocated) {
    
    ufree ((char *) t_handle->entry);
    t_handle->entry_allocated = FALSE;

  } /* if (t_handle->entry_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeTrackHeader()
 *
 * frees the header struct and arrays
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeTrackHeader(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{

  if (t_handle->header_allocated) {

    ufree ((char *) t_handle->header);
    t_handle->header_allocated = FALSE;

  } /* if (t_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeSimpleTrackParams()
 *
 * frees the parameters for simple tracks
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeSimpleTrackParams(track_file_handle_t *t_handle,
			    const char *calling_routine)
     
{

  if (t_handle->simple_params_allocated) {
    
    ufree ((char *) t_handle->simple_params);
    t_handle->simple_params_allocated = FALSE;
    
  } /* if (t_handle->simple_params_allocated) */
  
  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeComplexTrackParams()
 *
 * frees the parameters for complex tracks
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeComplexTrackParams(track_file_handle_t *t_handle,
			     const char *calling_routine)
     
{

  if (t_handle->complex_params_allocated) {
    
    ufree ((char *) t_handle->complex_params);
    t_handle->complex_params_allocated = FALSE;
    
  } /* if (t_handle->complex_params_allocated) */
  
  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeTrackScanEntries()
 *
 * frees scan entries for a track
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeTrackScanEntries(track_file_handle_t *t_handle,
			   const char *calling_routine)
     
{
  
  if (t_handle->scan_entries_allocated) {
    
    ufree ((char *) t_handle->scan_entries);
    t_handle->scan_entries_allocated = FALSE;
    t_handle->n_scan_entries_allocated = 0;
    
  } /* if (t_handle->entry_allocated) */
  
  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfTrackReinit
 *
 * Reinitializes track index structs and arrays for storm_track use
 *
 **************************************************************************/

/*ARGSUSED*/

int RfTrackReinit(track_file_handle_t *t_handle,
		  const char *calling_routine)
     
{

  if (t_handle->header_allocated) {
    memset(t_handle->header, 0, sizeof(track_file_header_t));
  }

  if (t_handle->complex_params_allocated) {
    memset(t_handle->complex_params, 0, sizeof(complex_track_params_t));
  }

  if (t_handle->simple_params_allocated) {
    memset(t_handle->simple_params, 0, sizeof(simple_track_params_t));
  }

  if (t_handle->entry_allocated) {
    memset(t_handle->entry, 0, sizeof(track_file_entry_t));
  }
    
  if (t_handle->n_complex_allocated > 0) {
    memset(t_handle->complex_track_nums, 0,
	   t_handle->n_complex_allocated * sizeof(si32));
  }

  if (t_handle->n_scan_entries_allocated > 0) {
    memset(t_handle->scan_entries, 0,
	   t_handle->n_scan_entries * sizeof(track_file_entry_t));
  }
  
  if (t_handle->n_scans_allocated > 0) {
    memset(t_handle->scan_index, 0,
	   t_handle->n_scans_allocated * sizeof(track_file_scan_index_t));
  }

  if (t_handle->n_simple_allocated > 0) {
    memset (t_handle->simple_track_offsets, 0,
	    t_handle->n_simple_allocated * sizeof(si32));
    memset (t_handle->nsimples_per_complex, 0,
	    t_handle->n_simple_allocated * sizeof(si32));
    memset (t_handle->simples_per_complex_offsets, 0,
	    t_handle->n_simple_allocated * sizeof(si32));
    memset (t_handle->complex_track_offsets, 0,
	    t_handle->n_simple_allocated * sizeof(si32));
  }
    
  if (t_handle->track_utime_allocated) {
    memset(t_handle->track_utime, 0,
	   t_handle->n_track_utime_allocated * sizeof(track_utime_t));
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfOpenTrackFiles()
 *
 * Opens the track header and data files
 *
 * The track header file path must have been set
 *
 **************************************************************************/

#define THIS_ROUTINE "RfOpenTrackFiles"

int RfOpenTrackFiles(track_file_handle_t *t_handle,
		     const char *mode,
		     const char *header_file_path,
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

  RfCloseTrackFiles(t_handle, calling_sequence);

  /*
   * open the header file
   */
  
  if ((t_handle->header_file =
       Rf_fopen_uncompress((char *) header_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:RfOpenTrackFiles\n", calling_routine);
    fprintf(stderr, "Cannot open track header file '%s'\n",
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
   
    if (RfReadTrackHeader(t_handle, calling_sequence))
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
	      t_handle->header->data_file_name);
    } else {
      strcpy(data_file_path, t_handle->header->data_file_name);
    }

  } else {

    /*
     * file opened for writing, use ext to compute file name
     */

    if (data_file_ext == NULL) {
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
      fprintf(stderr,
	      "Must provide data file extension for file creation\n");
      return (R_FAILURE);
    }

    strncpy(tmp_path, header_file_path, MAX_PATH_LEN);
    
    if ((chptr = strrchr(tmp_path, '.')) == NULL) {
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
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
  
  if ((t_handle->data_file =
       Rf_fopen_uncompress(data_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:RfOpenTrackFiles\n", calling_routine);
    fprintf(stderr, "Cannot open track data file '%s'\n",
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
    strcpy(file_label, t_handle->header_file_label);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		t_handle->header_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing track header file label.\n");
      perror(t_handle->header_file_path);
      return (R_FAILURE);
      
    }

    /*
     * data file
     */

    memset ((void *) file_label,
	    (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(file_label, t_handle->data_file_label);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		t_handle->data_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing track data file label.\n");
      perror(t_handle->data_file_path);
      return (R_FAILURE);
      
    }

  } /* if (*mode == 'w') */

  /*
   * set header file path in index
   */
  
  if (t_handle->header_file_path != NULL)
    ufree ((char *) t_handle->header_file_path);

  t_handle->header_file_path = (char *) umalloc
    ((ui32) (strlen(header_file_path)+ 1));

  strcpy(t_handle->header_file_path, header_file_path);

  /*
   * set data file path in index
   */
  
  if (t_handle->data_file_path != NULL)
    ufree ((char *) t_handle->data_file_path);

  t_handle->data_file_path = (char *) umalloc
    ((ui32) (strlen(data_file_path)+ 1));

  strcpy(t_handle->data_file_path, data_file_path);

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadTrackEntry()
 *
 * reads in an entry for a track
 *
 * Note - space for the entry structure is allocated if entry_allocated
 * is TRUE. If not, previous allocation is assumed.
 *
 * If first is set to TRUE, then the first entry is read in. If not
 * the next entry is read in.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadTrackEntry"

int RfReadTrackEntry(track_file_handle_t *t_handle,
		     const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 offset;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate as necessary
   */
  
  if (!t_handle->entry_allocated)
    if (RfAllocTrackEntry(t_handle, calling_sequence))
      return (R_FAILURE);
  /*
   * move to the entry offset in the file
   */
  
  if (t_handle->first_entry) {
    offset = t_handle->simple_params->first_entry_offset;
    t_handle->first_entry = FALSE;
  } else {
    offset = t_handle->entry->next_entry_offset;
  }
  
  fseek(t_handle->data_file, offset, SEEK_SET);
  
  /*
   * read in entry
   */
  
  if (ufread((char *) t_handle->entry, sizeof(track_file_entry_t),
	     1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackEntry\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track entry, simple track num %d.\n",
	    t_handle->simple_params->simple_track_num);
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->entry,
	    (ui32) sizeof(track_file_entry_t));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadTrackHeader()
 *
 * reads in the track_file_header_t structure from a storm track
 * file.
 *
 * Note - space for the header is allocated if header_allocated is FALSE.
 * If not, previous allocation is assumed.
 *
 * If the file label is passed in as NULL, the track file label
 * from the file is stored. If the label is non-null, the two are
 * compared and an error is returned if they are different.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadTrackHeader"

int RfReadTrackHeader(track_file_handle_t *t_handle,
		      const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[R_FILE_LABEL_LEN];
  si32 nbytes_char;
  si32 n_complex_tracks;
  si32 n_simple_tracks;
  si32 n_scans;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate space for header and arrays
   */
  
  if (!t_handle->header_allocated)
    if (RfAllocTrackHeader(t_handle, calling_sequence))
      return (R_FAILURE);
  
  /*
   * rewind file
   */
  
  fseek(t_handle->header_file, 0L, SEEK_SET);
  
  /*
   * read in file label
   */
  
  if (ufread(file_label, (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     t_handle->header_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading file label.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  
  /*
   * if the label passed in is non-null, check that the label matches
   * that expected. If the label is null, store the file label there.
   */
  
  if (t_handle->header_file_label != NULL) {
    
    if (strcmp(file_label, t_handle->header_file_label)) {
      fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "File does not contain correct type track data file.\n");
      fprintf(stderr, "File label is '%s'\n", file_label);
      fprintf(stderr, "File label should be '%s'.\n",
	      t_handle->header_file_label);
      
      return (R_FAILURE);
      
    }
    
  } else {
    
    t_handle->header_file_label = (char *) ucalloc
      ((ui32) (R_FILE_LABEL_LEN), (ui32) sizeof(char));
    
    strcpy(t_handle->header_file_label, file_label);
    
  } /* if (t_handle->header_file_label != NULL) */
  
  /*
   * read in header
   */
  
  if (ufread((char *) t_handle->header, sizeof(track_file_header_t),
	     1, t_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file header structure.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the structure into host byte order - the file
   * is stored in network byte order
   */
  
  nbytes_char = t_handle->header->nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) t_handle->header,
	    (ui32) (sizeof(track_file_header_t) - nbytes_char));
  
  n_complex_tracks = t_handle->header->n_complex_tracks;
  n_simple_tracks = t_handle->header->n_simple_tracks;
  n_scans = t_handle->header->n_scans;
  
  /*
   * check that the constants in use when the file was written are
   * less than or the same as those in use now
   */
  
  if (RfCheckTrackMaxValues(t_handle,
			    calling_sequence,
			    t_handle->header->max_parents,
			    (si32) MAX_PARENTS,
			    "max_parents",
			    "MAX_PARENTS")) {
    return (R_FAILURE);
  }
  
  if (RfCheckTrackMaxValues(t_handle,
			    calling_sequence,
			    t_handle->header->max_children,
			    (si32) MAX_CHILDREN,
			    "max_children",
			    "MAX_CHILDREN")) {
    return (R_FAILURE);
  }
  
  if (RfCheckTrackMaxValues(t_handle,
			    calling_sequence,
			    t_handle->header->max_nweights_forecast,
			    (si32) MAX_NWEIGHTS_FORECAST,
			    "max_nweights_forecast",
			    "MAX_NWEIGHTS_FORECAST")) {
    return (R_FAILURE);
  }

  /*
   * alloc header arrays
   */

  if (RfAllocTrackComplexArrays(t_handle,
				n_complex_tracks,
				calling_sequence))
    return (R_FAILURE);
  
  if (RfAllocTrackSimpleArrays(t_handle,
			       n_simple_tracks,
			       calling_sequence))
    return (R_FAILURE);
  
  if (RfAllocSimplesPerComplex(t_handle,
			       n_simple_tracks,
			       calling_sequence))
    return (R_FAILURE);
  
  if (RfAllocTrackScanIndex(t_handle,
			    n_scans,
			    calling_sequence))
    return (R_FAILURE);
  
  /*
   * read in complex track num array
   */

  if (ufread((char *) t_handle->complex_track_nums,
	     sizeof(si32),
	     n_complex_tracks,
	     t_handle->header_file) != n_complex_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file complex track num array.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->complex_track_nums,
	    (si32) (n_complex_tracks * sizeof(si32)));
  
  /*
   * read in complex track offset
   */
  
  if (ufread((char *) t_handle->complex_track_offsets,
	     sizeof(si32),
	     n_simple_tracks,
	     t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file complex_track offsets.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->complex_track_offsets,
	    (si32) (n_simple_tracks * sizeof(si32)));
  
  /*
   * read in simple track offset
   */
  
  if (ufread((char *) t_handle->simple_track_offsets,
	     sizeof(si32),
	     n_simple_tracks,
	     t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file simple_track offsets.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->simple_track_offsets,
	    (si32) (n_simple_tracks * sizeof(si32)));
  
  /*
   * read in scan index array
   */
  
  if (ufread((char *) t_handle->scan_index,
	     sizeof(track_file_scan_index_t),
	     n_scans,
	     t_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file scan index.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->scan_index,
	    (si32) (n_scans * sizeof(track_file_scan_index_t)));
  
  /*
   * read in nsimples_per_complex
   */
  
  if (ufread((char *) t_handle->nsimples_per_complex,
	     sizeof(si32),
	     n_simple_tracks,
	     t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file nsimples_per_complex.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->nsimples_per_complex,
	    (si32) (n_simple_tracks * sizeof(si32)));
  
  /*
   * read in simples_per_complex_offsets
   */
  
  if (ufread((char *) t_handle->simples_per_complex_offsets,
	     sizeof(si32),
	     n_simple_tracks,
	     t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading track file simples_per_complex_offsets.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->simples_per_complex_offsets,
		 (si32) (n_simple_tracks * sizeof(si32)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadComplexTrackParams()
 *
 * reads in the parameters for a complex track
 *
 * Note - space for the params structure is allocated if
 * params_allocated is FALSE. If not, previous allocation is assumed.
 *
 * For normal reads, read_simples_per_complex should be set TRUE. This
 * is only set FALSE in storm_track, which creates the track files.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadComplexTrackParams"

int RfReadComplexTrackParams(track_file_handle_t *t_handle,
			     si32 track_num,
			     int read_simples_per_complex,
			     const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nsimples;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * read in params
   */
  
  /*
   * allocate as necessary
   */
  
  if (!t_handle->complex_params_allocated)
    if (RfAllocComplexTrackParams(t_handle, calling_sequence))
      return (R_FAILURE);
  
  /*
   * move to offset in file
   */
  
  if (t_handle->complex_track_offsets[track_num] == 0) {
    return (R_FAILURE);
  }
  
  fseek(t_handle->data_file, t_handle->complex_track_offsets[track_num],
	SEEK_SET);
  
  /*
   * read in params
   */
  
  if (ufread((char *) t_handle->complex_params,
	     sizeof(complex_track_params_t),
	     1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s\n",
	    t_handle->prog_name, calling_sequence);
    fprintf(stderr, "Reading complex track params, track num %d.\n",
	    track_num);
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) t_handle->complex_params,
		 (ui32) sizeof(complex_track_params_t));

  /*
   * If read_simples_per_complex is set,
   * read in simples_per_complex array, which indicates which
   * simple tracks are part of this complex track.
   */

  if (read_simples_per_complex) {

    nsimples = t_handle->nsimples_per_complex[track_num];

    if (t_handle->simples_per_complex[track_num] == NULL) {
      t_handle->simples_per_complex[track_num] = (si32 *) umalloc
	(nsimples * sizeof(si32));
    } else {
      t_handle->simples_per_complex[track_num] = (si32 *) urealloc
	(t_handle->simples_per_complex[track_num],
	 nsimples * sizeof(si32));
    }
  
    fseek(t_handle->header_file,
	  t_handle->simples_per_complex_offsets[track_num], SEEK_SET);
  
    if (ufread((char *) t_handle->simples_per_complex[track_num],
	       sizeof(si32), nsimples,
	       t_handle->header_file) != nsimples) {
      
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
      fprintf(stderr, "Reading simples_per_complex for "
	      "complex track params, track num %d.\n", track_num);
      perror(t_handle->header_file_path);
      return (R_FAILURE);
      
    }
    
    BE_to_array_32((ui32 *) t_handle->simples_per_complex[track_num],
		   nsimples * sizeof(si32));

  } /*   if (read_simples_per_complex) */
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadSimplesPerComplex()
 *
 * reads in the array of simple tracks for each complex track
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadSimplesPerComplex"

int RfReadSimplesPerComplex(track_file_handle_t *t_handle,
			    const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 itrack, nsimples;
  si32 complex_num;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  for (itrack = 0; itrack < t_handle->header->n_complex_tracks; itrack++) {

    complex_num = t_handle->complex_track_nums[itrack];
    nsimples = t_handle->nsimples_per_complex[complex_num];
  
    t_handle->simples_per_complex[complex_num] = (si32 *) urealloc
      ((char *) t_handle->simples_per_complex[complex_num],
       (nsimples * sizeof(si32)));

    fseek(t_handle->header_file,
	  t_handle->simples_per_complex_offsets[complex_num], SEEK_SET);
    
    if (ufread((char *) t_handle->simples_per_complex[complex_num],
	       sizeof(si32), nsimples,
	       t_handle->header_file) != nsimples) {
      
      fprintf(stderr, "ERROR - %s:%s\n",
	      t_handle->prog_name, calling_sequence);
      fprintf(stderr, "Reading simples_per_complex for "
	      "complex track params, track num %d.\n", complex_num);
      perror(t_handle->header_file_path);
      return (R_FAILURE);
      
    }
    
    BE_to_array_32((ui32 *) t_handle->simples_per_complex[complex_num],
		   nsimples * sizeof(si32));

  } /* itrack */
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadSimpleTrackParams()
 *
 * reads in the parameters for a simple track
 *
 * Note - space for the params structure is allocated if
 * params_allocated is FALSE. If not, previous allocation is assumed.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadSimpleTrackParams"

int RfReadSimpleTrackParams(track_file_handle_t *t_handle, si32 track_num,
			    const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * read in params
   */
  
  /*
   * allocate as necessary
   */
  
  if (!t_handle->simple_params_allocated)
    if (RfAllocSimpleTrackParams(t_handle, calling_sequence))
      return (R_FAILURE);
  
  /*
   * move to offset in file
   */
  
  fseek(t_handle->data_file, t_handle->simple_track_offsets[track_num],
	SEEK_SET);
  
  /*
   * read in params
   */
  
  if (ufread((char *) t_handle->simple_params,
	     sizeof(simple_track_params_t),
	     1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadSimpleTrackParams\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading simple track params, track num %d.\n",
	    track_num);
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the structure into host byte order - the file
   * is stored in network byte order
   */
  
  BE_to_array_32((ui32 *) t_handle->simple_params,
		 (ui32) sizeof(simple_track_params_t));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadTrackScanEntries()
 *
 * reads in entries for a scan
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadTrackScanEntries"

int RfReadTrackScanEntries(track_file_handle_t *t_handle,
			   si32 scan_num,
			   const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  si32 ientry;
  si32 next_entry_offset;
  track_file_entry_t *entry;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate as necessary
   */

  t_handle->n_scan_entries = t_handle->scan_index[scan_num].n_entries;
  
  if (RfAllocTrackScanEntries(t_handle, calling_sequence))
    return (R_FAILURE);

  entry = t_handle->scan_entries;
  next_entry_offset = t_handle->scan_index[scan_num].first_entry_offset;
  
  for (ientry = 0; ientry < t_handle->n_scan_entries; ientry++) {
    
    /*
     * move to the next entry offset in the file
     */
    
    fseek(t_handle->data_file, next_entry_offset, SEEK_SET);
  
    /*
     * read in entry
     */
  
    if (ufread((char *) entry, sizeof(track_file_entry_t),
	       1, t_handle->data_file) != 1) {
    
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr, "Reading track entry %ld, scan %ld.\n",
	      (long) ientry, (long) scan_num);
      perror(t_handle->data_file_path);
      return (R_FAILURE);
    
    }
  
    /*
     * decode the structure into host byte order - the file
     * is stored in network byte order
     */
    
    BE_to_array_32((ui32 *) entry,
	      (ui32) sizeof(track_file_entry_t));

    next_entry_offset = entry->next_scan_entry_offset;

    entry++;

  } /* ientry */
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadTrackUtime()
 *
 * creates an array of track_utime_t structs, reads the track file to
 * set the times. Array pointer is in track file index.
 *
 * Returns R_SUCCESS or R_FAILURE
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadTrackUtime"

int RfReadTrackUtime(track_file_handle_t *t_handle,
		     const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  si32 itrack;
  si32 complex_track_num;
  si32 simple_track_num;

  si32 start_time, end_time;

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
     
  RfAllocTrackUtime(t_handle, calling_sequence);
  
  /*
   * read the complex and simple track params and load up
   * the start and end julian time arrays - these are used to
   * determine if a track is a valid candidate for display
   */
      
  for (itrack = 0;
       itrack < t_handle->header->n_complex_tracks; itrack++) {
    
    complex_track_num = t_handle->complex_track_nums[itrack];
    
    if (RfReadComplexTrackParams(t_handle, complex_track_num,
				 TRUE, calling_sequence)) {
      return (R_FAILURE);
    }
    
    start_time = t_handle->complex_params->start_time;
    end_time = t_handle->complex_params->end_time;

    t_handle->track_utime[complex_track_num].start_complex = start_time;
    t_handle->track_utime[complex_track_num].end_complex = end_time;

  } /* itrack */
  
  for (itrack = 0;
       itrack < t_handle->header->n_simple_tracks; itrack++) {
    
    simple_track_num = itrack;
    
    if (RfReadSimpleTrackParams(t_handle, simple_track_num,
				calling_sequence)) {
      return (R_FAILURE);
    }
    
    start_time = t_handle->simple_params->start_time;
    end_time = t_handle->simple_params->end_time;
    
    t_handle->track_utime[simple_track_num].start_simple = start_time;
    t_handle->track_utime[simple_track_num].end_simple = end_time;

  } /* itrack */

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReuseTrackComplexSlot()
 *
 * Sets a complex params slot in the file available for
 * reuse, by setting the offset to its negative value.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfReuseTrackComplexSlot(track_file_handle_t *t_handle,
			    si32 track_num,
			    const char *calling_routine)
     
{

  si32 *offset;
  offset = t_handle->complex_track_offsets + track_num;

  if (*offset <= 0) {
    fprintf(stderr, "ERROR - %s:RfReuseTrackComplexSlot\n",
	    calling_routine);
    fprintf(stderr, "Slot for track num %ld not available.\n",
	    (long) track_num);
    return (R_FAILURE);
  }

  *offset *= -1;
  
  if (track_num < t_handle->lowest_avail_complex_slot)
    t_handle->lowest_avail_complex_slot = track_num;

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfRewindSimpleTrack()
 *
 * prepares a simple track for reading by reading in the simple track
 * params and setting the first_entry flag
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfRewindSimpleTrack"

int RfRewindSimpleTrack(track_file_handle_t *t_handle, si32 track_num,
			const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * read in simple track params
   */

  if (RfReadSimpleTrackParams(t_handle, track_num,
			      calling_sequence)) {

    return (R_FAILURE);

  }

  /*
   * set first_entry flag
   */

  t_handle->first_entry = TRUE;

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfRewriteTrackEntry()
 *
 * rewrites an entry for a track in the track data file
 *
 * The entry is written at the file offset of the original entry
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfRewriteTrackEntry"

int RfRewriteTrackEntry(track_file_handle_t *t_handle,
			const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  track_file_entry_t entry;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * copy entry to local variable
   */
  
  memcpy ((void *) &entry,
          (void *) t_handle->entry,
          (size_t) sizeof(track_file_entry_t));
  
  /*
   * move to entry offset
   */
  
  fseek(t_handle->data_file, entry.this_entry_offset, SEEK_SET);
  
  /*
   * code the structure into network byte order
   */
  
  BE_to_array_32((ui32 *) &entry,
	    (ui32) sizeof(track_file_entry_t));
  
  /*
   * write entry
   */
  
  if (ufwrite((char *) &entry, sizeof(track_file_entry_t),
	      1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:RfRewriteTrackEntry\n",
	    calling_sequence);
    fprintf(stderr, "Writing track entry.\n");
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfSeekEndTrackData()
 *
 * seeks to the end of the track file data
 *
 **************************************************************************/

int RfSeekEndTrackData(track_file_handle_t *t_handle,
		       const char *calling_routine)
     
{
  
  if (fseek(t_handle->data_file, 0L, SEEK_END) != 0) {
    
    fprintf(stderr, "ERROR - %s:%s:RfSeekEndTrackData\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Failed on seek.\n");
    perror(t_handle->data_file_path);
    
    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * RfSeekStartTrackData()
 *
 * seeks to the start of data in track data file
 *
 **************************************************************************/

int RfSeekStartTrackData(track_file_handle_t *t_handle,
			 const char *calling_routine)
     
{

  if (fseek(t_handle->data_file, R_FILE_LABEL_LEN,
	    SEEK_SET) != 0) {
    
    fprintf(stderr, "ERROR - %s:%s:RfSeekStartTrackData\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Failed on seek.\n");
    perror(t_handle->data_file_path);
    
    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * RfWriteTrackEntry()
 *
 * writes in an entry for a track in the track data file
 *
 * The entry is written at the end of the file
 *
 * returns offset of last entry written on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteTrackEntry"

si32 RfWriteTrackEntry(track_file_handle_t *t_handle,
		       si32 prev_in_track_offset,
		       si32 prev_in_scan_offset,
		       const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 file_mark;
  track_file_entry_t entry;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * Go to the end of the file and save the file position.
   */

  fseek(t_handle->data_file, 0, SEEK_END);
  file_mark = ftell(t_handle->data_file);

  /*
   * If prev_in_track_offset is non-zero (which indicates that this is not
   * the first entry in a track) read in the entry at that location,
   * update the next_entry prev_in_track_offset with the current file
   * location and write back to file
   */
  
  if (prev_in_track_offset != 0) {
    
    /*
     * move to the entry prev_in_track_offset in the file
     */
    
    fseek(t_handle->data_file, prev_in_track_offset, SEEK_SET);
    
    /*
     * read in entry
     */
    
    if (ufread((char *) &entry,
	       sizeof(track_file_entry_t),
	       1, t_handle->data_file) != 1) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteTrackEntry\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading track entry to update in_track_offset\n.");
      perror(t_handle->data_file_path);
      return ((si32) R_FAILURE);
      
    }
    
    /*
     * store next_entry_offset
     */
    
    entry.next_entry_offset = file_mark;
    
    BE_from_array_32((ui32 *) &entry.next_entry_offset,
		(ui32) sizeof(entry.next_entry_offset));
    
    /*
     * move back to offset
     */
    
    fseek(t_handle->data_file, prev_in_track_offset, SEEK_SET);
    
    /*
     * rewrite entry
     */
    
    if (ufwrite((char *) &entry, sizeof(track_file_entry_t),
		1, t_handle->data_file) != 1) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteTrackEntry\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr, "Re-writing track entry.\n");
      perror(t_handle->data_file_path);
      return ((si32) R_FAILURE);
      
    }
    
  } /* if (prev_in_track_offset == 0) */
  
  /*
   * If prev_in_scan_offset is non-zero (which indicates that this is not
   * the first entry in a track) read in the entry at that location,
   * update the next_scan_entry_offset with the current file
   * location and write back to file
   */
  
  if (prev_in_scan_offset != 0) {
    
    /*
     * move to the entry prev_in_scan_offset in the file
     */
    
    fseek(t_handle->data_file, prev_in_scan_offset, SEEK_SET);
    
    /*
     * read in entry
     */
    
    if (ufread((char *) &entry,
	       sizeof(track_file_entry_t),
	       1, t_handle->data_file) != 1) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteTrackEntry\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading track entry to update in_track_offset\n.");
      perror(t_handle->data_file_path);
      return ((si32) R_FAILURE);
      
    }
    
    /*
     * store next_entry_offset
     */
    
    entry.next_scan_entry_offset = file_mark;
    
    BE_from_array_32((ui32 *) &entry.next_scan_entry_offset,
		(ui32) sizeof(entry.next_scan_entry_offset));
    
    /*
     * move back to offset
     */
    
    fseek(t_handle->data_file, prev_in_scan_offset, SEEK_SET);
    
    /*
     * rewrite entry
     */
    
    if (ufwrite((char *) &entry, sizeof(track_file_entry_t),
		1, t_handle->data_file) != 1) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteTrackEntry\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr, "Re-writing track entry.\n");
      perror(t_handle->data_file_path);
      return ((si32) R_FAILURE);
      
    }
    
  } /* if (prev_in_scan_offset == 0) */
  
  /*
   * go to end of file to write entry structure
   */
  
  fseek(t_handle->data_file, 0, SEEK_END);
  
  /*
   * copy entry to local variable
   */
  
  memcpy ((void *) &entry,
          (void *) t_handle->entry,
          (size_t) sizeof(track_file_entry_t));
  
  /*
   * set entry offsets
   */
  
  entry.prev_entry_offset = prev_in_track_offset;
  entry.this_entry_offset = file_mark;
  entry.next_entry_offset = 0;
  
  BE_from_array_32((ui32 *) &entry,
	    (ui32) sizeof(track_file_entry_t));
  
  /*
   * write entry
   */
  
  if (ufwrite((char *) &entry, sizeof(track_file_entry_t),
	      1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackEntry\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track entry.\n");
    perror(t_handle->data_file_path);
    return ((si32) R_FAILURE);
    
  }
  
  return (file_mark);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfWriteTrackHeader()
 *
 * writes the track_file_header_t structure to a track data file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteTrackHeader"

int RfWriteTrackHeader(track_file_handle_t *t_handle,
		       const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char label[R_FILE_LABEL_LEN];

  si32 n_complex_tracks;
  si32 n_simple_tracks;
  si32 n_scans;
  si32 icomplex;
  si32 complex_num, nsimples;
  si32 offsets_pos;

  si32 *complex_track_nums;
  si32 *complex_track_offsets;
  si32 *simple_track_offsets;
  si32 *nsimples_per_complex;
  si32 *simples_per_complex;
  si32 *simples_per_complex_offsets;

  track_file_scan_index_t *scan_index;
  track_file_header_t header;
  struct stat data_stat;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * get data file size
   */

  fflush(t_handle->data_file);
  stat (t_handle->data_file_path, &data_stat);
  t_handle->header->data_file_size = data_stat.st_size;
  
  /*
   * set file time to gmt
   */
  
  t_handle->header->file_time = time((time_t *) NULL);
  
  /*
   * copy file label
   */
  
  memset ((void *) label,
          (int) 0, (size_t)  R_FILE_LABEL_LEN);
  strcpy(label, t_handle->header_file_label);
  
  t_handle->header->major_rev = TRACK_FILE_MAJOR_REV_V5;
  t_handle->header->minor_rev = TRACK_FILE_MINOR_REV_V5;
  
  /*
   * move to start of file and write label
   */
  
  fseek(t_handle->header_file, (si32) 0, SEEK_SET);
  
  if (ufwrite(label, sizeof(char), R_FILE_LABEL_LEN,
	      t_handle->header_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file label.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * allocate local arrays
   */

  n_complex_tracks = t_handle->header->n_complex_tracks;
  n_simple_tracks = t_handle->header->n_simple_tracks;
  n_scans = t_handle->header->n_scans;
  
  complex_track_nums = (si32 *) umalloc
    ((ui32) (n_complex_tracks * sizeof(si32)));

  complex_track_offsets = (si32 *) umalloc
    ((ui32) (n_simple_tracks * sizeof(si32)));

  simple_track_offsets = (si32 *) umalloc
    ((ui32) (n_simple_tracks * sizeof(si32)));

  scan_index = (track_file_scan_index_t *) umalloc
    ((ui32) (n_scans * sizeof(track_file_scan_index_t)));

  simples_per_complex_offsets = (si32 *) umalloc
    ((ui32) (n_simple_tracks * sizeof(si32)));
  
  nsimples_per_complex = (si32 *) umalloc
    ((ui32) (n_simple_tracks * sizeof(si32)));
  
  simples_per_complex = (si32 *) umalloc
    ((ui32) (n_simple_tracks * sizeof(si32)));
  
  /*
   * copy the header and arrays to local variables
   */
  
  memcpy ((void *)  &header,
          (void *)  t_handle->header,
          (size_t)  sizeof(track_file_header_t));
  
  memcpy ((void *)  complex_track_nums,
          (void *)  t_handle->complex_track_nums,
          (size_t)  (n_complex_tracks *  sizeof(si32)));
  
  memcpy ((void *)  complex_track_offsets,
          (void *)  t_handle->complex_track_offsets,
          (size_t)  (n_simple_tracks *  sizeof(si32)));
  
  memcpy ((void *)  simple_track_offsets,
          (void *)  t_handle->simple_track_offsets,
          (size_t)  (n_simple_tracks *  sizeof(si32)));
  
  memcpy ((void *)  scan_index,
          (void *)  t_handle->scan_index,
          (size_t)  (n_scans *  sizeof(track_file_scan_index_t)));
  
  memcpy ((void *)  nsimples_per_complex,
          (void *)  t_handle->nsimples_per_complex,
          (size_t)  (n_simple_tracks *  sizeof(si32)));
  
  /*
   * code into network byte order
   */
  
  header.nbytes_char = (TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN +
                        TRACK_FILE_HEADER_NBYTES_CHAR);
			  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.storm_header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.verify.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32((ui32 *) &header,
	      (ui32) (sizeof(track_file_header_t) - header.nbytes_char));
  
  BE_from_array_32((ui32 *) complex_track_nums,
	      (si32) (n_complex_tracks *  sizeof(si32)));
  
  BE_from_array_32((ui32 *) complex_track_offsets,
	      (si32) (n_simple_tracks *  sizeof(si32)));
  
  BE_from_array_32((ui32 *) simple_track_offsets,
	      (si32) (n_simple_tracks *  sizeof(si32)));
  
  BE_from_array_32((ui32 *) scan_index,
	      (si32) (n_scans *  sizeof(track_file_scan_index_t)));
  
  BE_from_array_32((ui32 *) nsimples_per_complex,
		   (si32) (n_simple_tracks *  sizeof(si32)));
  
  /*
   * write header
   */
  
  if (ufwrite((char *) &header,
	      sizeof(track_file_header_t),
	      1, t_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file header structure.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write in complex track num array
   */
  
  if (ufwrite((char *) complex_track_nums,
	      sizeof(si32),
	      n_complex_tracks,
	      t_handle->header_file) != n_complex_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file complex track num array.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write in complex track offset
   */
  
  if (ufwrite((char *) complex_track_offsets,
	      sizeof(si32),
	      n_simple_tracks,
	      t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file complex track offsets.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write in simple track offset
   */
  
  if (ufwrite((char *) simple_track_offsets,
	      sizeof(si32),
	      n_simple_tracks,
	      t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file simple track offsets.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write in scan index
   */
  
  if (ufwrite((char *) scan_index,
	      sizeof(track_file_scan_index_t),
	      n_scans,
	      t_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file scan index.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }

  /*
   * write in nsimples_per_complex
   */

  if (ufwrite((char *) nsimples_per_complex,
	      sizeof(si32),
	      n_simple_tracks,
	      t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing track file nsimples_per_complex.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  offsets_pos = ftell(t_handle->header_file);
  
  /*
   * seek ahead of the simples_per_complex_offsets array
   */

  fseek(t_handle->header_file,
	n_simple_tracks * sizeof(si32), SEEK_CUR);

  /*
   * clear offsets array
   */

  memset((void *) simples_per_complex_offsets, 0,
	 n_simple_tracks * sizeof(si32));
  
  /*
   * loop through complex tracks
   */

  for (icomplex = 0; icomplex < n_complex_tracks; icomplex++) {

    complex_num = t_handle->complex_track_nums[icomplex];
    nsimples = t_handle->nsimples_per_complex[complex_num];
    simples_per_complex_offsets[complex_num] = ftell(t_handle->header_file);
    
    memcpy((void *) simples_per_complex,
	   t_handle->simples_per_complex[complex_num],
	   (nsimples * sizeof(si32)));

    BE_from_array_32((ui32 *) simples_per_complex,
		     (nsimples * sizeof(si32)));
		     
    /*
     * write out simple tracks array
     */
    
    if (ufwrite((char *) simples_per_complex,
		sizeof(si32), nsimples,
		t_handle->header_file) != nsimples) {
      
      fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	      t_handle->prog_name, calling_routine);
      fprintf(stderr, "Writing simples_per_complex array.\n");
      perror(t_handle->header_file_path);
      return (R_FAILURE);
      
    }
    
  } /* icomplex */

  /*
   * write out simples_per_complex_offsets
   */
  
  fseek(t_handle->header_file, offsets_pos, SEEK_SET);
  BE_from_array_32((ui32 *) simples_per_complex_offsets,
		   (n_simple_tracks * sizeof(si32)));
  
  if (ufwrite((char *) simples_per_complex_offsets,
	      sizeof(si32), n_simple_tracks,
	      t_handle->header_file) != n_simple_tracks) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteTrackHeader\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing simples_per_complex_offsets array.\n");
    perror(t_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * flush the file buffer
   */
  
  if (RfFlushTrackFiles(t_handle, calling_sequence)) {
    return (R_FAILURE);
  }

  /*
   * free resources
   */

  ufree(complex_track_nums);
  ufree(complex_track_offsets);
  ufree(simple_track_offsets);
  ufree(nsimples_per_complex);
  ufree(simples_per_complex_offsets);
  ufree(simples_per_complex);
  ufree(scan_index);

  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfWriteSimpleTrackParams()
 *
 * writes the parameters for a simple track at the end of the file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteSimpleTrackParams(track_file_handle_t *t_handle,
			     si32 track_num,
			     const char *calling_routine)
     
{
  
  int rewrite;

  si32 file_mark;
  
  simple_track_params_t simple_params;

  /*
   * Go to the end of the file.
   */

  fseek(t_handle->data_file, 0, SEEK_END);
  file_mark = ftell(t_handle->data_file);

  /*
   * if params have been written before, go to the stored offset.
   * If not, store offset as current file location
   */
  
  if (t_handle->simple_track_offsets[track_num] == 0) {
    
    t_handle->simple_track_offsets[track_num] = file_mark;
    rewrite = FALSE;
    
  } else {
    
    rewrite = TRUE;
    
  }
  
  /*
   * copy track params, encode and write to file
   */
  
  memcpy ((void *)  &simple_params,
	  (void *)  t_handle->simple_params,
	  (size_t) 
	  sizeof(simple_track_params_t));
  
  BE_from_array_32((ui32 *) &simple_params,
		   (ui32) sizeof(simple_track_params_t));
  
  /*
   * for rewrite, move to stored offset
   */
  
  if (rewrite) {
    fseek(t_handle->data_file, t_handle->simple_track_offsets[track_num],
	  SEEK_SET);
  }
  
  if (ufwrite((char *) &simple_params,
	      sizeof(simple_track_params_t),
	      1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteSimpleTrackParams\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing simple track params, track num %d.\n",
	    track_num);
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * flush the file buffer
   */
  
  if (fflush(t_handle->data_file) != 0) {
    fprintf(stderr, "ERROR - %s:%s:RfWriteSimpleTrackParams\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot flush buffer.\n");
    perror(t_handle->data_file_path);
    return (R_FAILURE);
  }
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfWriteComplexTrackParams()
 *
 * writes the parameters for a complex track
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteComplexTrackParams(track_file_handle_t *t_handle,
			      si32 track_num,
			      const char *calling_routine)
     
{
  
  int slot_found;

  si32 i;
  si32 lowest_avail_slot;
  si32 avail_offset;
  si32 *offset_p;
  
  complex_track_params_t complex_params;

  if (t_handle->complex_track_offsets[track_num] == 0) {
    
    /*
     * params have not been written before.
     *
     * Two steps: 1) look for a slot which has been freed
     *               up when a complex track was consolidated.
     *            2) If no available slot, use end of file
     */
    
    lowest_avail_slot = t_handle->lowest_avail_complex_slot;
    offset_p = t_handle->complex_track_offsets + lowest_avail_slot;
    slot_found = FALSE;
    
    for (i = lowest_avail_slot; i < track_num; i++) {
      
      if (*offset_p < 0) {
	
	/*
	 * avail slot found
	 */
	
	avail_offset = -(*offset_p);
	*offset_p = 0;
	t_handle->lowest_avail_complex_slot = i + 1;
	slot_found = TRUE;
	break;
	
      } /* if (*offset_p < 0) */
      
      offset_p++;
      
    } /* i */
    
    if (slot_found) {
      
      t_handle->complex_track_offsets[track_num] = avail_offset;
      fseek(t_handle->data_file, avail_offset, SEEK_SET);
      
    } else {
      
      fseek(t_handle->data_file, 0, SEEK_END);
      t_handle->complex_track_offsets[track_num] = ftell(t_handle->data_file);
      t_handle->lowest_avail_complex_slot = track_num + 1;
      
    }
    
  } else {
    
    /*
     * params have been stored before, so go to the stored offset
     */
    
    fseek(t_handle->data_file, t_handle->complex_track_offsets[track_num],
	  SEEK_SET);
    
  }
  
  /*
   * copy track params, encode and write to file
   */
  
  memcpy ((void *) &complex_params,
	  (void *) t_handle->complex_params,
	  (size_t) 
	  sizeof(complex_track_params_t));
  
  BE_from_array_32((ui32 *) &complex_params,
		   (ui32) sizeof(complex_track_params_t));
  
  if (ufwrite((char *) &complex_params,
	      sizeof(complex_track_params_t),
	      1, t_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteComplexTrackParams\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing complex track params, track num %d.\n",
	    track_num);
    perror(t_handle->data_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * flush the file buffer
   */
  
  if (fflush(t_handle->data_file) != 0) {
    fprintf(stderr, "ERROR - %s:%s:RfWriteComplexTrackParams\n",
	    t_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot flush buffer.\n");
    perror(t_handle->data_file_path);
    return (R_FAILURE);
  }
  
  return (R_SUCCESS);
  
}

